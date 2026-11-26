# Zero-Migration Strangler-Fig Hybrid Workspace Pattern (`npm` / `pnpm` / `holo-pm`)

> **Reference Finding:** `pnpm/pnpm#3367` — Decoupling physical Content-Addressable Storage (CAS) hardlink materialization from metadata-only `package-lock.json` projection.

---

## 1. Why Separate Physical CAS Storage from Metadata Lockfile Projection?

Enterprise monorepos and legacy CI pipelines frequently depend on `package-lock.json` for vulnerability scanning (SCA/SBOM), deterministic audit gates, or downstream `npm ci` consumers. Attempting a "big-bang" rewrite away from `npm` stalls adoption, while running standard `npm install` destroys deduplication by copying trees into `node_modules`.

The **Zero-Migration Strangler-Fig Hybrid Workspace Pattern** resolves this by separating two concerns:

1. **Physical Central CAS Storage (`pnpm` global store / `holo-v11` CAS):**
   - Package tarballs and extracted files live once in the shared `v11` CAS (`v11/files/00..ff/<sha512>`) or `pnpm` content-addressable store.
   - Workspace `node_modules` and `./bin/` projections are materialized as **0-byte hardlinks** (`project://`), eliminating disk duplication and multi-minute copy loops.
2. **Metadata-Only Lockfile Projection (`--package-lock-only` / `holo-ci lock-compat`):**
   - `npm i --package-lock-only` computes and writes `package-lock.json` purely as metadata **without** writing, extracting, or overwriting `node_modules` on disk.
   - Enterprise scanners, legacy `npm` tooling, and `holo-v11` CAS workspaces operate side-by-side over the same workspace tree with zero `node_modules` copies.

---

## 2. Awesome-OS `bin-shim` Pass-Through Behavior

Under `@awesome-os/bin-shim`, invocations of `npm`, `yarn`, and `corepack` are intercepted and routed to `pnpm` (`npx` is rewritten to `pnx`) so physical installs always stay on the central CAS store.

The single intentional exception is metadata-only lockfile projection:

- When invoked with `--package-lock-only`, `E:\awesome-os\bin\npm.cmd` (`resolvePackageManagerTarget` in `pnpm-shim.mts`) enables **`lockfileOnlyProjection: true`** and passes the command directly through to `npm`.
- Physical `node_modules` remain backed by central `pnpm` / `holo-v11` CAS hardlinks while `package-lock.json` is updated cleanly.

---

## 3. Step-by-Step Hybrid Workspace Recipe

Run the following sequence from the repository root to attach a new `holo-pm` / `pnpm` workspace lane alongside existing `npm` metadata consumers:

### Step 1 — Scaffold the Strangler-Fig Workspace Lane

```powershell
npm init -w ./workspaces/holo-packages
```

Creates `./workspaces/holo-packages/package.json` and adds `"workspaces": ["workspaces/holo-packages"]` to the root `package.json` without migrating existing legacy packages.

### Step 2 — Materialize Central CAS Hardlinks

```powershell
pnpm install
```

Resolves the workspace dependency graph into the shared central CAS store and links `node_modules` via 0-byte hardlinks (no physical package copies).

### Step 3 — Project Metadata-Only `package-lock.json`

```powershell
# Standard invocation (routed through the workspace npm shim):
npm i --package-lock-only

# Explicit Awesome-OS Windows shim pass-through:
E:\awesome-os\bin\npm.cmd i --package-lock-only
```

Writes or refreshes `package-lock.json` in metadata-only mode while leaving the CAS-hardlinked `node_modules` untouched.

### Step 4 — Verify Lockfile & CAS Parity in CI

```powershell
holo-ci lock-compat
```

Validates lockfile compatibility across `package-lock.json`, `pnpm-lock.yaml`, and the `holo-v11` CAS index before merge or release.

---

## 4. Operational Invariants

- **Never run bare `npm install` without `--package-lock-only`** in a CAS-managed workspace; physical tree materialization must go through `pnpm install` or `holo-pm install`.
- **Keep `repository://` as the Source of Truth:** Commit lockfiles and pin manifests ([`pins/holo-unified-npm-overrides.json`](../pins/holo-unified-npm-overrides.json)), not materialized `node_modules`.
- **Incremental Strangler-Fig Adoption:** Move packages into `./workspaces/holo-packages` one slice at a time while legacy CI stages continue consuming the projected `package-lock.json`.

---

## Related References

- [`README.md`](../README.md) — `holo-pm` (`libholo-pm`) 2-protocol model, `./bin/` hardlinks, and `CAS Q-Self` overview.
- [`pins/holo-unified-npm-overrides.json`](../pins/holo-unified-npm-overrides.json) — Wave-1 unified npm dependency overrides.
- [`index/holo-compiler-lockfile.json`](../index/holo-compiler-lockfile.json) — Git-versioned `v11` CAS compiler and runtime lockfile.

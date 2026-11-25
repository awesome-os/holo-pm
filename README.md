# holo-pm (`libholo-pm`) — Unified SDK & Self-Hosted `v11` CAS Package Manager

**`holo-pm`** (`libholo-pm`) is the unified Awesome-OS SDK home, Git-versioned package index, and self-hosted Layer-0 `v11` CAS package manager unifying `libpnpm`, `libcargo`, `libuvpy`, and `libuv` behind a single `B8G_ABI_MAGIC_V3` (`0x33564742`) C11 facade.

It implements the **2-protocol model**:
- **`repository://`** — Immutable **Source of Truth** (Git refs, `v11/files/00..ff/<sha512>` CAS blobs, and lockfiles).
- **`project://`** — **0-byte Virtual Worktree / Install** projected via content-addressed hardlinks into `./bin/` by default.

---

## Quick Bootstrap (Windows / Linux / macOS)

```powershell
# Windows (PowerShell) — 1-line bootstrap install
irm https://github.com/awesome-os/holo-pm/releases/latest/download/install.ps1 | iex
```

```sh
# Linux & macOS — 1-line bootstrap install
curl -fsSL https://github.com/awesome-os/holo-pm/releases/latest/download/install.sh | sh
```

---

## Package Installs: Local-by-Default `./bin/` & Collision-Guarded `-g`

Every `holo-pm install` resolves artifacts from `repository://` into the shared `v11` CAS and projects them into the current `project://` worktree's `./bin/` directory using **0-byte hardlinks** by default.

Global installs (`-g`) are **collision-guarded**: if a binary name already exists in the global `bin/` directory, `holo-pm` refuses to overwrite it unless you supply `--as <name>` to install under a distinct alias or `--force-default` to explicitly replace the default command.

```sh
# Local-by-default ./bin/ hardlink install (0-byte project:// projection)
holo-pm install repository://github.com/awesome-os/holo-pm@main
holo-pm install repository://github.com/awesome-os/holo-jq@main
holo-pm install repository://github.com/awesome-os/coreutils@main

# Collision-guarded global install
holo-pm install -g repository://github.com/awesome-os/holo-jq@main

# Resolve a global command collision via explicit alias or override
holo-pm install -g repository://github.com/awesome-os/coreutils@main --as awesome-coreutils
holo-pm install -g repository://github.com/awesome-os/coreutils@main --force-default
```

---

## Unified SDK Index, Wave-1 Pins & `CAS Q-Self` Fixed-Point Proof

`holo-pm` is the canonical Git-versioned home for the Awesome-OS `v11` CAS index, cross-language dependency pins, and self-hosting proof:

| Path | Purpose |
| :--- | :--- |
| [`index/holo-compiler-lockfile.json`](index/holo-compiler-lockfile.json) | Git-versioned `v11` CAS lockfile pinning compiler, host, and runtime seeds (`sha512`). |
| [`index/cas-v11-global-manifest.json`](index/cas-v11-global-manifest.json) · [`index/SHA512SUMS.txt`](index/SHA512SUMS.txt) | Global `v11` CAS manifest and cryptographic digest ledger. |
| [`pins/`](pins/) | Wave-1 unified dependency pins across Go ([`holo-unified-go-pins.mod`](pins/holo-unified-go-pins.mod), [`holo-unified-go.work`](pins/holo-unified-go.work)), npm ([`holo-unified-npm-overrides.json`](pins/holo-unified-npm-overrides.json)), and Rust ([`holo-unified-rust-pins.toml`](pins/holo-unified-rust-pins.toml)). |
| [`bootstrap/selfhost-fixedpoint.mts`](bootstrap/selfhost-fixedpoint.mts) | **`CAS Q-Self` Fixed-Point Verifier** — Stage 0 -> Stage 1 -> Stage 2 proof that `sha512(Stage 1) === sha512(Stage 2)` with `deduplicated === true` (`0` extra bytes in `v11`). |

```sh
# Run the Stage 0 -> Stage 1 -> Stage 2 CAS Q-Self fixed-point proof
node --test bootstrap/selfhost-fixedpoint.test.mts
```

---

## Native C11 ABI Exports (`include/libholo_pm.h`)

- `B8G_HOLO_PM_ABI_MAGIC_V3` (`0x33564742u`)
- `uint32_t b8g_holo_pm_abi_magic(void)`
- `const char* b8g_holo_pm_version(void)` (`"0.1.0+b8g-cas-v11"`)
- `int32_t b8g_holo_pm_resolve_v11_root(char* out, size_t cap)`
- `int32_t b8g_holo_pm_probe_lib(const char* lib_path, const char* magic_sym, uint32_t* out_magic)`

## Native CLI Inspection (`holo-pm`)

```sh
# Inspect ABI magic, version, CAS v11 root, OS, and arch
holo-pm --info

# Dynamically load and verify any B8G_ABI_MAGIC_V3 shard
holo-pm --probe ./libpnpm.dll b8g_pnpm_abi_magic
holo-pm --probe ./libcargo.dll b8g_cargo_abi_magic
holo-pm --probe ./libuvpy.dll b8g_uvpy_abi_magic
```

## Build (Zero Dependencies, `zig cc`)

```sh
# Shared library
zig cc -std=c11 -Os -s -Wall -Wextra -Werror -DB8G_HOLO_PM_BUILDING_DLL -Iinclude -shared src/libholo_pm.c -o libholo-pm.dll

# Native CLI
zig cc -std=c11 -Os -s -Wall -Wextra -Werror -Iinclude src/libholo_pm.c src/holo_pm.c -o holo-pm.exe

# Test suite
zig cc -std=c11 -Os -s -Wall -Wextra -Werror -Iinclude src/libholo_pm.c src/holo_pm_test.c -o holo_pm_test.exe
```

## License

MIT — see [LICENSE](LICENSE).

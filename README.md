# holo-pm (`libholo-pm`)

Layer-0 native CAS v11 package-manager seed unifying `libpnpm`, `libcargo`, `libuvpy`, and `libuv` behind a single `B8G_ABI_MAGIC_V3` (`0x33564742`) C11 facade.

## Exports (`include/libholo_pm.h`)

- `B8G_HOLO_PM_ABI_MAGIC_V3` (`0x33564742u`)
- `uint32_t b8g_holo_pm_abi_magic(void)`
- `const char* b8g_holo_pm_version(void)` (`"0.1.0+b8g-cas-v11"`)
- `int32_t b8g_holo_pm_resolve_v11_root(char* out, size_t cap)`
- `int32_t b8g_holo_pm_probe_lib(const char* lib_path, const char* magic_sym, uint32_t* out_magic)`

## CLI (`holo-pm`)

```bash
# Inspect ABI magic, version, CAS v11 root, OS, and arch
holo-pm --info

# Dynamically load and verify any B8G_ABI_MAGIC_V3 shard
holo-pm --probe ./libpnpm.dll b8g_pnpm_abi_magic
holo-pm --probe ./libcargo.dll b8g_cargo_abi_magic
holo-pm --probe ./libuvpy.dll b8g_uvpy_abi_magic
```

## Build (Zero Dependencies, `zig cc`)

```bash
# Shared library
zig cc -std=c11 -Os -s -Wall -Wextra -Werror -DB8G_HOLO_PM_BUILDING_DLL -Iinclude -shared src/libholo_pm.c -o libholo-pm.dll

# Native CLI
zig cc -std=c11 -Os -s -Wall -Wextra -Werror -Iinclude src/libholo_pm.c src/holo_pm.c -o holo-pm.exe

# Test suite
zig cc -std=c11 -Os -s -Wall -Wextra -Werror -Iinclude src/libholo_pm.c src/holo_pm_test.c -o holo_pm_test.exe
```

## License

MIT — see [LICENSE](LICENSE).

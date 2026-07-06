# wasm-component-inspector

A small CLI that inspects a `.wasm` module using [WasmEdge](https://wasmedge.org)'s
C API: it walks the module through the runtime's three pipeline stages
(**loader → validator → executor**), prints the import/export sections with
pretty-printed types, and reports exactly which stage rejects an invalid file
and why.

```
$ ./wasm-inspector tests/add.wasm
wasm-component-inspector (WasmEdge 0.14.1)
inspecting: tests/add.wasm

[loader]    OK — binary format is well-formed

imports (0):

exports (2):
  [0] func   "add" : (i32, i32) -> (i32)
  [1] memory "mem"

[validator] OK — module obeys the type rules
[executor]  OK — module instantiated (imports resolved)
```

## Why per-stage reporting

A Wasm runtime rejects a bad module at one of three distinct stages, and the
stage tells you *what kind* of bad it is:

| stage | what it checks | example failure | WasmEdge code range |
|---|---|---|---|
| **loader** | binary format: magic, sections, LEB128 | `magic header not detected` | `0x1xx` |
| **validator** | type rules on the AST: stack effects, index bounds | `type mismatch` (value stack underflow) | `0x2xx` |
| **executor** | instantiation: import resolution, allocation, start fn | `unknown import` | `0x3xx` |

Most tools collapse these into a single "invalid module" error. This one keeps
them apart, and prints the section listing *before* validation — so you can see
what a module *claims* to contain even when it is rejected.

## Build

Requires WasmEdge (tested with 0.14.1) installed at `~/.wasmedge` (the
default location of the [official install script](https://wasmedge.org/docs/start/install)),
and a C compiler.

```
make
./wasm-inspector <module.wasm>
```

Exit code is `0` if all stages pass, `1` otherwise.

## Test fixtures

Hand-written binaries (see the hex in `tests/`), one per failure class:

| fixture | outcome |
|---|---|
| `tests/add.wasm` | valid: exports `add : (i32, i32) -> (i32)` and a memory |
| `tests/empty.wasm` | valid: minimal 8-byte module, all stages pass |
| `tests/garbage.wasm` | fails **loader**: not a wasm binary |
| `tests/bad-types.wasm` | fails **validator**: function promises `i32`, body returns nothing |
| `tests/needs-import.wasm` | fails **executor**: imports `env.foo`, nothing provides it |
| `tests/imports.wasm` | fails **executor**: imports a func, a memory and a global |

## Layout

```
src/main.c                   CLI entry point
src/inspector.h              public interface (no WasmEdge types)
src/pipeline.c               the three pipeline stages + error reporting
src/print.h                  internal printing interface
src/print_types.c            value-type and function-signature printers
src/print_imports_exports.c  import/export section walkers
```

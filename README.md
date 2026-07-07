# wasm-component-inspector

A small CLI that inspects a `.wasm` module using [WasmEdge](https://wasmedge.org)'s
C API. I built this to properly understand what a Wasm runtime does between
"here are some bytes" and "here is a running module", specifically the loader,
validator and executor stages.

The tool walks a module through all three stages explicitly, prints the
import/export sections with their types, and when a file is broken it tells you
which stage rejected it and why, instead of a generic "invalid module" error.

```
$ ./wasm-inspector tests/add.wasm
wasm-component-inspector (WasmEdge 0.14.1)
inspecting: tests/add.wasm

[loader]    OK, binary format is well-formed

imports (0):

exports (2):
  [0] func   "add" : (i32, i32) -> (i32)
  [1] memory "mem"

[validator] OK, module obeys the type rules
[executor]  OK, module instantiated (imports resolved)
```

## Build and run

You need WasmEdge (I used 0.14.1) installed at `~/.wasmedge`, the default
location of the [official install script](https://wasmedge.org/docs/start/install),
and a C compiler.

```
make
./wasm-inspector <module.wasm>
```

Exit code is `0` if all stages pass, `1` otherwise.

## The pipeline, as I understand it now

The thing that clicked for me while building this: the three stages answer
three different questions, and a module can pass one and still fail the next.

**The loader asks: are these bytes even a wasm binary?** It checks the magic
number (`\0asm`), the version, section sizes, and the LEB128 integer
encodings, then builds an in-memory AST. It never looks at meaning, only
shape, so a module full of type errors sails straight through it.

**The validator asks: does this module obey WebAssembly's rules?** It
simulates the stack effect of every instruction in every function body and
checks that every index is in bounds. My favourite demo is
`tests/bad-types.wasm`: a function whose signature promises an `i32` result
but whose body is just `end`. The loader accepts it because every byte is
structurally fine. The validator simulates the body, reaches `end` expecting
an `i32` on the stack, finds the stack empty, and rejects it. Validation is a
check, not a transformation: it returns no new object, but because the AST
comes out trusted, the executor never re-checks types at runtime.

**The executor asks: can this module actually come to life here?**
Instantiation allocates memories, tables and globals, resolves every import
against what the store already contains, runs initializers and the start
function, and returns a live instance. This is a third class of failure: a
perfectly valid module still fails if it imports `env.foo` and nobody
registered an `env` module. The store was a new concept for me; it is the
runtime universe that exists independently of any single module, and it is
what makes linking possible.

WasmEdge encodes the stage into its error codes, which the tool surfaces:

| stage | what it checks | example failure | code range |
|---|---|---|---|
| loader | binary format | `magic header not detected` | `0x1xx` |
| validator | type rules on the AST | `type mismatch` | `0x2xx` |
| executor | instantiation and imports | `unknown import` | `0x3xx` |

One deliberate choice: the tool prints imports/exports right after the loader,
before validation, because the AST is inspectable the moment it parses. You
see what a module claims to contain even when the validator is about to
reject it.

## Things I learned about the C API

- Everything is an opaque context with a Create/Delete pair. You own the
  pointer and must free it. I used a single `cleanup:` label and delete in
  reverse creation order (the instance lives in the store, so it goes first).
- Errors come back by value as `WasmEdge_Result`; you check it with
  `WasmEdge_ResultOK()`. The internal logger repeats the same information, so
  I turned it off with `WasmEdge_LogOff()`.
- The type section is not directly listable. Function types are only reachable
  through import/export entries, which store a type index that the AST
  resolves.
- Listing is a two-call pattern: `...Length()` tells you how many entries
  exist, `...List(buf, cap)` fills your buffer. The returned pointers are
  borrowed views, nothing to free.
- `WasmEdge_String` is not NUL-terminated; print it with `%.*s`.
- One honest C bug I hit: stage lines go to stdout, failures to stderr. Piped,
  stdout is block-buffered and stderr is not, so the `FAILED` line printed
  first. Fix: `fflush(stdout)` before writing the failure.

## Test fixtures

I wrote every fixture by hand with `printf`, byte by byte, instead of
compiling from `.wat`. Decoding the sections by hand taught me the binary
format properly. One fixture per failure class:

| fixture | outcome |
|---|---|
| `tests/add.wasm` | valid: exports `add : (i32, i32) -> (i32)` and a memory |
| `tests/empty.wasm` | valid: minimal 8-byte module |
| `tests/garbage.wasm` | fails **loader**: not a wasm binary |
| `tests/bad-types.wasm` | fails **validator**: promises `i32`, returns nothing |
| `tests/needs-import.wasm` | fails **executor**: imports `env.foo`, nothing provides it |
| `tests/imports.wasm` | fails **executor**: imports a func, a memory and a global |

For example, `bad-types.wasm` is 25 bytes:

```
00 61 73 6d 01 00 00 00   header: "\0asm" + version 1
01 05 01 60 00 01 7f      TYPE section: 1 type: () -> i32
03 02 01 00               FUNCTION section: 1 function, uses type #0
0a 04 01 02 00 0b         CODE section: 1 body: no locals, just `end`
```

## Layout

```
src/main.c                   CLI entry point
src/inspector.h              public interface (knows no WasmEdge types)
src/pipeline.c               the three pipeline stages + error reporting
src/print.h                  internal printing interface
src/print_types.c            value-type and function-signature printers
src/print_imports_exports.c  import/export section walkers
```

## Possible next steps

A `--json` output mode, full type detail on exports (currently only imports
get it), and registering the WASI host module so `wasi_snapshot_preview1`
imports resolve.

#ifndef INSPECTOR_H
#define INSPECTOR_H

// Public interface, deliberately free of WasmEdge types.

// Runs loader -> validator -> executor on the .wasm file at `path`
// and prints a report. Returns 0 if every stage passes, 1 otherwise.
int inspect_file(const char *path);

#endif

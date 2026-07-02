#ifndef INSPECTOR_H
#define INSPECTOR_H

/* Runs the WasmEdge pipeline (loader -> validator -> executor) on the
 * .wasm file at `path`, printing a report of what each stage finds.
 *
 * Returns 0 if every stage succeeds, 1 if any stage rejects the file. */
int inspect_file(const char *path);

#endif

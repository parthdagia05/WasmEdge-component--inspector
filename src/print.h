#ifndef PRINT_H
#define PRINT_H

/* Internal helpers for pretty-printing module contents.
 * Unlike inspector.h this header is allowed to know WasmEdge types. */

#include <wasmedge/wasmedge.h>

/* "i32", "f64", "funcref", ... */
const char *val_type_name(WasmEdge_ValType vt);

/* Prints a signature like "(i32, i32) -> (i32)" (no newline). */
void print_function_type(const WasmEdge_FunctionTypeContext *ft);

/* Walks the AST's import section and prints one line per import. */
void print_imports(const WasmEdge_ASTModuleContext *ast);

/* Walks the AST's export section and prints one line per export. */
void print_exports(const WasmEdge_ASTModuleContext *ast);

#endif

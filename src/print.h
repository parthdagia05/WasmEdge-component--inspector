#ifndef PRINT_H
#define PRINT_H

// Internal pretty-printing helpers, unlike inspector.h this header
// is allowed to know WasmEdge types.

#include <wasmedge/wasmedge.h>

// "i32", "f64", "funcref", ...
const char *val_type_name(WasmEdge_ValType vt);

// prints a signature like "(i32, i32) -> (i32)", no trailing newline
void print_function_type(const WasmEdge_FunctionTypeContext *ft);

// one line per import section entry
void print_imports(const WasmEdge_ASTModuleContext *ast);

// one line per export section entry
void print_exports(const WasmEdge_ASTModuleContext *ast);

#endif

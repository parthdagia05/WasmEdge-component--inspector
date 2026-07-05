#include <stdio.h>

#include "print.h"

#define MAX_ENTRIES 128

static const char *extern_kind_name(enum WasmEdge_ExternalType kind) {
    switch (kind) {
    case WasmEdge_ExternalType_Function: return "func";
    case WasmEdge_ExternalType_Table:    return "table";
    case WasmEdge_ExternalType_Memory:   return "memory";
    case WasmEdge_ExternalType_Global:   return "global";
    default:                             return "?";
    }
}

void print_exports(const WasmEdge_ASTModuleContext *ast) {
    const WasmEdge_ExportTypeContext *exports[MAX_ENTRIES];

    uint32_t total = WasmEdge_ASTModuleListExportsLength(ast);
    uint32_t got = WasmEdge_ASTModuleListExports(ast, exports, MAX_ENTRIES);

    printf("\nexports (%u):\n", total);
    for (uint32_t i = 0; i < got; i++) {
        enum WasmEdge_ExternalType kind =
            WasmEdge_ExportTypeGetExternalType(exports[i]);
        /* WasmEdge_String is NOT NUL-terminated: print via %.*s.
         * The name is a view into the AST — nothing to free. */
        WasmEdge_String name = WasmEdge_ExportTypeGetExternalName(exports[i]);

        printf("  [%u] %-6s \"%.*s\"", i, extern_kind_name(kind),
               (int)name.Length, name.Buf);

        if (kind == WasmEdge_ExternalType_Function) {
            const WasmEdge_FunctionTypeContext *ft =
                WasmEdge_ExportTypeGetFunctionType(ast, exports[i]);
            printf(" : ");
            print_function_type(ft);
        }
        printf("\n");
    }
    if (total > got) printf("  ... %u more not shown\n", total - got);
}

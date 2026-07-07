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

// memory limits are in 64KiB pages, table limits are entry counts
static void print_limits(WasmEdge_Limit lim) {
    if (lim.HasMax) {
        printf("{min %u, max %u%s}", lim.Min, lim.Max,
               lim.Shared ? ", shared" : "");
    } else {
        printf("{min %u}", lim.Min);
    }
}

void print_imports(const WasmEdge_ASTModuleContext *ast) {
    const WasmEdge_ImportTypeContext *imports[MAX_ENTRIES];

    uint32_t total = WasmEdge_ASTModuleListImportsLength(ast);
    // the List call returns the full length, not how many entries it
    // wrote into the buffer, so clamp before iterating
    uint32_t shown = total < MAX_ENTRIES ? total : MAX_ENTRIES;
    WasmEdge_ASTModuleListImports(ast, imports, MAX_ENTRIES);

    printf("\nimports (%u):\n", total);
    for (uint32_t i = 0; i < shown; i++) {
        const WasmEdge_ImportTypeContext *imp = imports[i];
        enum WasmEdge_ExternalType kind =
            WasmEdge_ImportTypeGetExternalType(imp);
        // imports are named "module.field", the module half is the
        // namespace the host must register (e.g. "env")
        WasmEdge_String mod = WasmEdge_ImportTypeGetModuleName(imp);
        WasmEdge_String name = WasmEdge_ImportTypeGetExternalName(imp);

        // WasmEdge_String is not NUL-terminated, hence %.*s
        printf("  [%u] %-6s \"%.*s\".\"%.*s\"", i, extern_kind_name(kind),
               (int)mod.Length, mod.Buf, (int)name.Length, name.Buf);

        // each import kind carries its own type detail
        switch (kind) {
        case WasmEdge_ExternalType_Function: {
            const WasmEdge_FunctionTypeContext *ft =
                WasmEdge_ImportTypeGetFunctionType(ast, imp);
            printf(" : ");
            print_function_type(ft);
            break;
        }
        case WasmEdge_ExternalType_Memory: {
            const WasmEdge_MemoryTypeContext *mt =
                WasmEdge_ImportTypeGetMemoryType(ast, imp);
            printf(" : pages ");
            print_limits(WasmEdge_MemoryTypeGetLimit(mt));
            break;
        }
        case WasmEdge_ExternalType_Table: {
            const WasmEdge_TableTypeContext *tt =
                WasmEdge_ImportTypeGetTableType(ast, imp);
            printf(" : %s ", val_type_name(WasmEdge_TableTypeGetRefType(tt)));
            print_limits(WasmEdge_TableTypeGetLimit(tt));
            break;
        }
        case WasmEdge_ExternalType_Global: {
            const WasmEdge_GlobalTypeContext *gt =
                WasmEdge_ImportTypeGetGlobalType(ast, imp);
            printf(" : %s %s",
                   WasmEdge_GlobalTypeGetMutability(gt) ==
                           WasmEdge_Mutability_Var
                       ? "mut"
                       : "const",
                   val_type_name(WasmEdge_GlobalTypeGetValType(gt)));
            break;
        }
        default:
            break;
        }
        printf("\n");
    }
    if (total > shown) printf("  ... %u more not shown\n", total - shown);
}

void print_exports(const WasmEdge_ASTModuleContext *ast) {
    const WasmEdge_ExportTypeContext *exports[MAX_ENTRIES];

    uint32_t total = WasmEdge_ASTModuleListExportsLength(ast);
    // same clamp as print_imports, the List call returns the full length
    uint32_t shown = total < MAX_ENTRIES ? total : MAX_ENTRIES;
    WasmEdge_ASTModuleListExports(ast, exports, MAX_ENTRIES);

    printf("\nexports (%u):\n", total);
    for (uint32_t i = 0; i < shown; i++) {
        enum WasmEdge_ExternalType kind =
            WasmEdge_ExportTypeGetExternalType(exports[i]);
        // the name is a borrowed view into the AST, nothing to free
        WasmEdge_String name = WasmEdge_ExportTypeGetExternalName(exports[i]);

        printf("  [%u] %-6s \"%.*s\"", i, extern_kind_name(kind),
               (int)name.Length, name.Buf);

        // only functions get a signature, other kinds just show their name
        if (kind == WasmEdge_ExternalType_Function) {
            const WasmEdge_FunctionTypeContext *ft =
                WasmEdge_ExportTypeGetFunctionType(ast, exports[i]);
            printf(" : ");
            print_function_type(ft);
        }
        printf("\n");
    }
    if (total > shown) printf("  ... %u more not shown\n", total - shown);
}

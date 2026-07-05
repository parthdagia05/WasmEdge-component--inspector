#include <stdio.h>

#include "print.h"

const char *val_type_name(WasmEdge_ValType vt) {
    if (WasmEdge_ValTypeIsI32(vt)) return "i32";
    if (WasmEdge_ValTypeIsI64(vt)) return "i64";
    if (WasmEdge_ValTypeIsF32(vt)) return "f32";
    if (WasmEdge_ValTypeIsF64(vt)) return "f64";
    if (WasmEdge_ValTypeIsV128(vt)) return "v128";
    if (WasmEdge_ValTypeIsFuncRef(vt)) return "funcref";
    if (WasmEdge_ValTypeIsExternRef(vt)) return "externref";
    return "?";
}

/* Fixed cap keeps this allocation-free; signatures wider than 16
 * params/results are vanishingly rare, and we print how many we hid. */
#define MAX_TYPES 16

static void print_type_list(const WasmEdge_ValType *types, uint32_t shown,
                            uint32_t total) {
    printf("(");
    for (uint32_t i = 0; i < shown; i++) {
        printf("%s%s", i > 0 ? ", " : "", val_type_name(types[i]));
    }
    if (total > shown) printf(", ... %u more", total - shown);
    printf(")");
}

void print_function_type(const WasmEdge_FunctionTypeContext *ft) {
    WasmEdge_ValType params[MAX_TYPES], results[MAX_TYPES];

    uint32_t nparams = WasmEdge_FunctionTypeGetParametersLength(ft);
    uint32_t nresults = WasmEdge_FunctionTypeGetReturnsLength(ft);
    uint32_t got_p = WasmEdge_FunctionTypeGetParameters(ft, params, MAX_TYPES);
    uint32_t got_r = WasmEdge_FunctionTypeGetReturns(ft, results, MAX_TYPES);

    print_type_list(params, got_p, nparams);
    printf(" -> ");
    print_type_list(results, got_r, nresults);
}

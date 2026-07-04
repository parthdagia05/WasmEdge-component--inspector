#include <stdio.h>

#include <wasmedge/wasmedge.h>

#include "inspector.h"

static void report_failure(const char *stage, WasmEdge_Result res) {
    fprintf(stderr, "FAILED at %s stage: %s (code 0x%03x)\n",
            stage, WasmEdge_ResultGetMessage(res), WasmEdge_ResultGetCode(res));
}

int inspect_file(const char *path) {
    int rc = 1;
    WasmEdge_ConfigureContext *conf = NULL;
    WasmEdge_LoaderContext *loader = NULL;
    WasmEdge_ValidatorContext *validator = NULL;
    WasmEdge_ASTModuleContext *ast = NULL;

    conf = WasmEdge_ConfigureCreate();
    loader = WasmEdge_LoaderCreate(conf);
    validator = WasmEdge_ValidatorCreate(conf);
    if (loader == NULL || validator == NULL) {
        fprintf(stderr, "error: failed to create loader/validator\n");
        goto cleanup;
    }

    /* Stage 1: LOADER — parse the raw bytes into an AST module.
     * This only checks the *binary format*: magic number, section
     * layout, LEB128 encodings. No type rules yet. */
    WasmEdge_Result res = WasmEdge_LoaderParseFromFile(loader, &ast, path);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("loader", res);
        goto cleanup;
    }
    printf("[loader]    OK — binary format is well-formed\n");

    /* Stage 2: VALIDATOR — check the AST against WebAssembly's type
     * rules: every instruction's stack effect, index bounds (types,
     * funcs, tables, memories, globals), import/export sanity.
     * A module can parse fine and still die here. */
    res = WasmEdge_ValidatorValidate(validator, ast);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("validator", res);
        goto cleanup;
    }
    printf("[validator] OK — module obeys the type rules\n");

    rc = 0;

cleanup:
    if (ast != NULL) WasmEdge_ASTModuleDelete(ast);
    if (validator != NULL) WasmEdge_ValidatorDelete(validator);
    if (loader != NULL) WasmEdge_LoaderDelete(loader);
    if (conf != NULL) WasmEdge_ConfigureDelete(conf);
    return rc;
}

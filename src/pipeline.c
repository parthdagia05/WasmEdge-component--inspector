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
    WasmEdge_ASTModuleContext *ast = NULL;

    conf = WasmEdge_ConfigureCreate();
    loader = WasmEdge_LoaderCreate(conf);
    if (loader == NULL) {
        fprintf(stderr, "error: failed to create loader\n");
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

    rc = 0;

cleanup:
    if (ast != NULL) WasmEdge_ASTModuleDelete(ast);
    if (loader != NULL) WasmEdge_LoaderDelete(loader);
    if (conf != NULL) WasmEdge_ConfigureDelete(conf);
    return rc;
}

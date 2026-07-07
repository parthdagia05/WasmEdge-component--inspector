#include <stdio.h>

#include <wasmedge/wasmedge.h>

#include "inspector.h"
#include "print.h"

static void report_failure(const char *stage, WasmEdge_Result res) {
    // flush stdout first: it is block-buffered when piped, stderr is not,
    // so without this the failure line can jump ahead of the stage lines
    fflush(stdout);
    fprintf(stderr, "FAILED at %s stage: %s (code 0x%03x)\n",
            stage, WasmEdge_ResultGetMessage(res), WasmEdge_ResultGetCode(res));
}

int inspect_file(const char *path) {
    int rc = 1;
    WasmEdge_LoaderContext *loader = NULL;
    WasmEdge_ValidatorContext *validator = NULL;
    WasmEdge_ExecutorContext *executor = NULL;
    WasmEdge_StoreContext *store = NULL;
    WasmEdge_ASTModuleContext *ast = NULL;
    WasmEdge_ModuleInstanceContext *instance = NULL;

    // NULL means default configuration for all three stages
    loader = WasmEdge_LoaderCreate(NULL);
    validator = WasmEdge_ValidatorCreate(NULL);
    executor = WasmEdge_ExecutorCreate(NULL, NULL);
    store = WasmEdge_StoreCreate();
    if (loader == NULL || validator == NULL || executor == NULL ||
        store == NULL) {
        fprintf(stderr, "error: failed to create pipeline contexts\n");
        goto cleanup;
    }

    // stage 1: loader parses the bytes into an AST, this checks binary
    // format only (magic number, section layout, LEB128), no type rules yet
    WasmEdge_Result res = WasmEdge_LoaderParseFromFile(loader, &ast, path);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("loader", res);
        goto cleanup;
    }
    printf("[loader]    OK, binary format is well-formed\n");

    // the AST is inspectable before validation, so we can show what the
    // binary claims to contain even if the validator rejects it next
    print_imports(ast);
    print_exports(ast);
    printf("\n");

    // stage 2: validator checks the AST against WebAssembly's type rules,
    // stack effects of every instruction and all index bounds
    res = WasmEdge_ValidatorValidate(validator, ast);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("validator", res);
        goto cleanup;
    }
    printf("[validator] OK, module obeys the type rules\n");

    // stage 3: executor instantiates the validated module in the store,
    // allocates memories/tables/globals and resolves every import, so a
    // valid module still fails here if its imports are unsatisfied
    res = WasmEdge_ExecutorInstantiate(executor, &instance, store, ast);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("executor", res);
        goto cleanup;
    }
    printf("[executor]  OK, module instantiated (imports resolved)\n");

    rc = 0;

cleanup:
    // delete in reverse creation order, the instance lives inside the store
    if (instance != NULL) WasmEdge_ModuleInstanceDelete(instance);
    if (store != NULL) WasmEdge_StoreDelete(store);
    if (executor != NULL) WasmEdge_ExecutorDelete(executor);
    if (ast != NULL) WasmEdge_ASTModuleDelete(ast);
    if (validator != NULL) WasmEdge_ValidatorDelete(validator);
    if (loader != NULL) WasmEdge_LoaderDelete(loader);
    return rc;
}

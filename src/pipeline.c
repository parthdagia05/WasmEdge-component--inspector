#include <stdio.h>

#include <wasmedge/wasmedge.h>

#include "inspector.h"
#include "print.h"

static void report_failure(const char *stage, WasmEdge_Result res) {
    /* stdout is block-buffered when piped, stderr is not; flush so the
     * failure line can never appear before the stages that preceded it. */
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

    loader = WasmEdge_LoaderCreate(NULL); /* NULL = default configuration */
    validator = WasmEdge_ValidatorCreate(NULL);
    executor = WasmEdge_ExecutorCreate(NULL, NULL);
    store = WasmEdge_StoreCreate();
    if (loader == NULL || validator == NULL || executor == NULL ||
        store == NULL) {
        fprintf(stderr, "error: failed to create pipeline contexts\n");
        goto cleanup;
    }

    /* Stage 1: LOADER: parse the raw bytes into an AST module.
     * This only checks the *binary format*: magic number, section
     * layout, LEB128 encodings. No type rules yet. */
    WasmEdge_Result res = WasmEdge_LoaderParseFromFile(loader, &ast, path);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("loader", res);
        goto cleanup;
    }
    printf("[loader]    OK, binary format is well-formed\n");

    /* The AST can be inspected as soon as the loader accepts it,
     * before validation. This shows what the binary *claims* to
     * contain, even for modules the validator will reject. */
    print_imports(ast);
    print_exports(ast);
    printf("\n");

    /* Stage 2: VALIDATOR: check the AST against WebAssembly's type
     * rules: every instruction's stack effect, index bounds (types,
     * funcs, tables, memories, globals), import/export sanity.
     * A module can parse fine and still die here. */
    res = WasmEdge_ValidatorValidate(validator, ast);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("validator", res);
        goto cleanup;
    }
    printf("[validator] OK, module obeys the type rules\n");

    /* Stage 3: EXECUTOR: instantiate the validated module inside the
     * store: allocate its memories/tables/globals, resolve every
     * import against what the store already holds, run the start
     * function. A perfectly valid module still fails here if its
     * imports are unsatisfied. */
    res = WasmEdge_ExecutorInstantiate(executor, &instance, store, ast);
    if (!WasmEdge_ResultOK(res)) {
        report_failure("executor", res);
        goto cleanup;
    }
    printf("[executor]  OK, module instantiated (imports resolved)\n");

    rc = 0;

cleanup:
    if (instance != NULL) WasmEdge_ModuleInstanceDelete(instance);
    if (store != NULL) WasmEdge_StoreDelete(store);
    if (executor != NULL) WasmEdge_ExecutorDelete(executor);
    if (ast != NULL) WasmEdge_ASTModuleDelete(ast);
    if (validator != NULL) WasmEdge_ValidatorDelete(validator);
    if (loader != NULL) WasmEdge_LoaderDelete(loader);
    return rc;
}

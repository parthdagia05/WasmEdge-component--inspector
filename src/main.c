#include <stdio.h>

#include <wasmedge/wasmedge.h>

int main(void) {
    printf("wasm-component-inspector (WasmEdge %s)\n",
           WasmEdge_VersionGet());

    WasmEdge_ConfigureContext *conf = WasmEdge_ConfigureCreate();
    if (conf == NULL) {
        fprintf(stderr, "error: failed to create WasmEdge configuration\n");
        return 1;
    }

    printf("configure context created OK\n");

    WasmEdge_ConfigureDelete(conf);
    return 0;
}

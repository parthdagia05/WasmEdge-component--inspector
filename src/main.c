#include <stdio.h>

#include <wasmedge/wasmedge.h>

#include "inspector.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <module.wasm>\n", argv[0]);
        return 2;
    }

    // silence WasmEdge's internal logger, our per-stage report says the same
    WasmEdge_LogOff();

    printf("wasm-component-inspector (WasmEdge %s)\n", WasmEdge_VersionGet());
    printf("inspecting: %s\n\n", argv[1]);

    return inspect_file(argv[1]);
}

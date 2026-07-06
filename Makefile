# where wasmedge lives
WASMEDGE_DIR = $(HOME)/.wasmedge

CC      = cc
# tells the compiler where to find wasmedge/wasmedge.h
CFLAGS  = -Wall -Wextra -g -I$(WASMEDGE_DIR)/include
# tells linker where libwasmedge.so is and to link it
LDFLAGS = -L$(WASMEDGE_DIR)/lib -lwasmedge -Wl,-rpath,$(WASMEDGE_DIR)/lib

SRCS = src/main.c src/pipeline.c src/print_types.c src/print_imports_exports.c
OBJS = $(SRCS:.c=.o)
BIN  = wasm-inspector

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# rebuild everything when a header changes
$(OBJS): src/print.h src/inspector.h

clean:
	rm -f $(OBJS) $(BIN)

.PHONY: all clean

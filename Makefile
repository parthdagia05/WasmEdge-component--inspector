WASMEDGE_DIR = $(HOME)/.wasmedge

CC      = cc
CFLAGS  = -Wall -Wextra -g -I$(WASMEDGE_DIR)/include
LDFLAGS = -L$(WASMEDGE_DIR)/lib -lwasmedge -Wl,-rpath,$(WASMEDGE_DIR)/lib

SRCS = src/main.c
OBJS = $(SRCS:.c=.o)
BIN  = wasm-inspector

$(BIN): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(BIN)

.PHONY: clean

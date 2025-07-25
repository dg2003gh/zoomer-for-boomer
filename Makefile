CC = cc
CFLAGS = -I include/headers -Wall -Wextra -O2
LDFLAGS = -lraylib -lX11 -lwayland-client -lpng -lm

SRC = src/main.c $(wildcard include/*.c)
BIN = dist/zfb

all: build

build: $(BIN)

$(BIN): $(SRC)
	@mkdir -p dist
	$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(BIN)

run: build
	./$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all build run clean

CC ?= cc
PREFIX ?= /usr/local
CFLAGS ?= -std=c17 -O2 -g -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wformat=2
LDFLAGS ?=

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%.c,build/%.o,$(SRC))
BIN := build/c2sh
TESTBIN := build/test_runner

.PHONY: all clean test sanitize install uninstall format-check

all: $(BIN)

$(BIN): $(OBJ) | build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ)

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -Iinclude -MMD -MP -c $< -o $@

build:
	mkdir -p build

-include $(OBJ:.o=.d)

test: $(BIN)
	$(CC) $(CFLAGS) -Iinclude tests/test_runner.c src/lexer.c src/parser.c src/expand.c src/util.c -o $(TESTBIN)
	./$(TESTBIN)
	sh tests/integration.sh ./$(BIN)

sanitize:
	$(MAKE) clean
	$(MAKE) CFLAGS='-std=c17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic' all test

format-check:
	@command -v clang-format >/dev/null 2>&1 && clang-format --dry-run --Werror src/*.c include/*.h || echo 'clang-format not installed; skipping format check'

install: $(BIN)
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(BIN) $(DESTDIR)$(PREFIX)/bin/c2sh
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 0644 docs/c2sh.1 $(DESTDIR)$(PREFIX)/share/man/man1/c2sh.1

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/c2sh
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/c2sh.1

clean:
	rm -rf build

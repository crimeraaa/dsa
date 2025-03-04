CC := clang

DEBUG_FLAGS := -fsanitize=address -O0 -g
RELEASE_FLAGS := -O1 -g
CC_FLAGS := -std=c11 -Wall -Wextra -Wconversion -pedantic

SRC := $(wildcard *.c) $(wildcard types/*.c)

debug: CC_FLAGS += $(DEBUG_FLAGS)
debug: main

main: $(SRC)
	$(CC) $(CC_FLAGS) -o $@ $^

release: CC_FLAGS += $(RELEASE_FLAGS)
release: main

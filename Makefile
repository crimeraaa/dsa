CC := clang

DEBUG_FLAGS := -fsanitize=address -O0 -g
RELEASE_FLAGS := -O1 -g

FLAGS := $(RELEASE_FLAGS)

# Do not use `:=` because we want lazy evaluation of `FLAGS`.
CC_FLAGS = -std=c11 -Wall -Wextra -Wconversion -pedantic $(FLAGS)

SRC := $(wildcard *.c)

main: $(SRC)
	$(CC) $(CC_FLAGS) -o $@ $^

debug: FLAGS := $(DEBUG_FLAGS)
debug: main

release: FLAGS := $(RELEASE_FLAGS)
release: main

%.c: %.h

CC := clang

DEBUG_FLAGS := -fsanitize=address -O0 -g
RELEASE_FLAGS := -O1 -s

CC_FLAGS := -std=c11 -Wall -Wextra -Wconversion -pedantic $(DEBUG_FLAGS)

SRC := $(wildcard *.c)

main: $(SRC)
	$(CC) $(CC_FLAGS) -o $@ $^

%.c: %.h

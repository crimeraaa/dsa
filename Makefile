CC := clang

DEBUG_FLAGS := -fsanitize=address -O0 -g
RELEASE_FLAGS := -O1

CC_FLAGS := -std=c11 -Wall -Wextra -Wconversion -pedantic $(RELEASE_FLAGS)

SRC := $(wildcard *.c)
HDR := $(wildcard *.h)

main: $(SRC)
	$(CC) $(CC_FLAGS) -o $@ $^

%.c: %.h

CC := gcc
CC_FLAGS := -std=c11 -Wall -Wextra -Wconversion -fsanitize=address -g -O0

SRC := $(wildcard *.c)
HDR := $(wildcard *.h)

main: $(SRC)
	$(CC) $(CC_FLAGS) -o $@ $^
	
%.c: %.h

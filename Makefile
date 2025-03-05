CC := clang

DEBUG_FLAGS := -fsanitize=address -O0 -g
RELEASE_FLAGS := -O1 -g
CC_FLAGS := -std=c11 -Wall -Wextra -Wconversion -pedantic

SOURCES := $(wildcard *.c) $(wildcard types/*.c)
HEADERS := $(wildcard *.h) $(wildcard types/*.h)

debug: CC_FLAGS += $(DEBUG_FLAGS)
debug: main

main: $(SOURCES) $(HEADERS)
	$(CC) $(CC_FLAGS) -o $@ $(SOURCES)

release: CC_FLAGS += $(RELEASE_FLAGS)
release: main

CC := clang

DEBUG_FLAGS := -fsanitize=address -O0 -g
RELEASE_FLAGS := -O1 -g
CC_FLAGS := -std=c11 -Wall -Wextra -Wconversion -pedantic

SOURCES := $(wildcard *.c) $(wildcard types/*.c)
HEADERS := $(wildcard *.h) $(wildcard types/*.h) $(wildcard mem/*.h)

debug: CC_FLAGS += $(DEBUG_FLAGS)
debug: main

main: $(SOURCES) $(HEADERS)
	$(CC) $(CC_FLAGS) -o $@ $(SOURCES)

.PHONY: test
test:
	clang++ -std=c++17 -Wall -Wextra -o test/a.out $(wildcard test/*.cpp)

release: CC_FLAGS += $(RELEASE_FLAGS)
release: main

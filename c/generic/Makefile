CC ?= gcc
CFLAGS ?= -O0 -std=c11 -Wall -Wextra -Werror -Wshadow -pedantic
CFLAGS += $(DEBUGFLAGS) # See ~/.config/dotfiles/config.gcc.mk

EXE = main
SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, obj/%.o, $(SRC))

all: bin/$(EXE)
	
bin obj:
	$(MKDIR) $@

bin/$(EXE): $(OBJ) | bin
	$(CC) $(CFLAGS) -o $@ $^
	
obj/%.o: %.c | obj
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	$(RM) obj/*.o bin/*

.PHONY: all clean

.PRECIOUS: obj/%.o

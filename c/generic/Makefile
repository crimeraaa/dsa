EXE = main

CC ?= gcc
CFLAGS ?= -O0 -std=c11 -Wall -Wextra -Werror -Wshadow -pedantic
CFLAGS += $(DEBUGFLAGS) # See ~/.config/dotfiles/config.gcc.mk

all: bin/$(EXE)
	
bin obj:
	$(MKDIR) $@

bin/$(EXE): obj/$(EXE).o obj/ga_array.o | bin
	$(CC) $(CFLAGS) -o $@ $^
	
obj/%.o: %.c | obj
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	$(RM) obj/*.o bin/*

.PHONY: all clean

.PRECIOUS: obj/%.o

CC 	   ?= gcc
CFLAGS ?= -O0 -std=c11 -Wall -Wextra -Werror -Wshadow -pedantic
CFLAGS += -fdiagnostics-color=always -g

all: bin/libsimple.so bin/main # For demonstration purposes this Makefile is rather verbose.

bin obj: # Called only for left-hand value that caused this recipe to be called.
	mkdir -p $@

bin/main: bin/libsimple.so | bin # Linker arguments are in that order specifically!
	$(CC) $(CFLAGS) -L./bin -Wl,-rpath=./bin/ -o $@ main.c -lsimple

bin/libsimple.so: obj/simple.o | bin
	$(CC) $(CFLAGS) -shared -o $@ $<

obj/simple.o: simple.c | obj # Can only mkdir obj when this target is called.
	$(CC) $(CFLAGS) -fPIC -shared -c -o $@ $<

clean:
	rm -f obj/simple.o bin/libsimple.so bin/main

.PHONY: all clean

.PRECIOUS: obj/simple.o bin/libsimple.so


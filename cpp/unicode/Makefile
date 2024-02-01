# For demonstration, this Makefile's recipes are rather verbose.

CFLAGS += $(DEBUGFLAGS) # See ~/.config/dotfiles/config.gcc.mk
CXXFLAGS += $(DEBUGFLAGS)

all: bin/c_test
	
bin obj:
	mkdir -p $@

bin/c_test: c_test.c obj/utf8.o | bin obj
	$(CC) $(CFLAGS) -o $@ $^
	
obj/utf8.o: utf8.cpp utf8.h | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	
clean:
	$(RM) obj/utf8.o bin/c_test
	
.PHONY: all clean

.PRECIOUS: obj/utf8.o

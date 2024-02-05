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

obj/%.o: %.cpp | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	$(RM) obj/*.o bin/*

.PHONY: all clean

.PRECIOUS: obj/%.o


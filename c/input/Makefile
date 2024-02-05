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
	$(RM) $(EXE) $(OBJ)
	
.PHONY: all clean

.PRECIOUS: obj/%.o

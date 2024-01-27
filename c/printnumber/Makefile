CFLAGS+=$(DEBUGFLAGS)
SRC=printnumber.c
EXE=$(patsubst %.c, ./bin/%, $(SRC))

all: $(EXE)
	
$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<
	
clean:
	$(RM) $(EXE)
	
.PHONY: all clean

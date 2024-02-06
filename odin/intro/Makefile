EXE = hello
SRC = $(wildcard *.odin)

# -vet enables: -vet-unused, -vet-shadowing and -vet-using-stmt
ODINFLAGS = -vet -warnings-as-errors

all: bin/$(EXE)
	
bin:
	$(MKDIR) $@

bin/$(EXE): $(SRC) | bin
	odin build . -out:$@ $(ODINFLAGS)
	
check:
	odin check . $(ODINFLAGS)

clean:
	$(RM) bin/$(EXE)

.PHONY: all check clean

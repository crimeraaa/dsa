# -*- BUILD DIRECTORIES ------------------------------------------------------*-

BINDIR=./bin
OBJDIR=./obj

# -*- SOURCE FILES -----------------------------------------------------------*-

SRC=$(wildcard *.cpp)
EXE=$(patsubst %.cpp, $(BINDIR)/%, $(SRC))

# -*- BEGIN:	RECIPES ------------------------------------------------------*-

all: $(EXE)

$(BINDIR) $(OBJDIR):
	$(MKDIR) $@

$(BINDIR)/%: %.cpp | $(BINDIR)
	$(CXX) $(DEBUGFLAGS) $(CXXFLAGS) -o $@ $<

print:
	@echo Directories: $(BINDIR) $(OBJDIR)
	@echo Sourcefiles: $(SRC)
	@echo Executables: $(EXE)

clean:
	$(RM) $(EXE)

.PHONY: all clean

# -*- END:		RECIPES ------------------------------------------------------*-

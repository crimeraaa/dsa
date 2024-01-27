# -*- BUILD DIRECTORIES ------------------------------------------------------*-

BINDIR=./bin
OBJDIR=./obj

# -*- SOURCE FILES -----------------------------------------------------------*-

CFLAGS+=$(DEBUGFLAGS)
CXXFLAGS+=$(DEBUGFLAGS)

PARSE=parse
PARSE_SRC=$(addsuffix .cpp, $(PARSE))
PARSE_OBJ=$(patsubst %, $(OBJDIR)/%.o, $(PARSE))

CMAIN=cmain
CMAIN_EXE=$(patsubst %, $(BINDIR)/%, $(CMAIN))
CMAIN_OBJ=$(patsubst %, $(OBJDIR)/%.o, $(CMAIN))

CPPMAIN=cppmain
CPPMAIN_EXE=$(patsubst %, $(BINDIR)/%, $(CPPMAIN))
CPPMAIN_OBJ=$(patsubst %, $(OBJDIR)/%.o, $(CPPMAIN))


# -*- BEGIN:	RECIPES ------------------------------------------------------*-

all: $(CMAIN_EXE) $(CPPMAIN_EXE) | $(BINDIR)
	
$(BINDIR) $(OBJDIR):
	$(MKDIR) $@
	
$(CMAIN_EXE): $(CMAIN_OBJ) $(PARSE_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(CPPMAIN_EXE): $(CPPMAIN_OBJ) $(PARSE_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^
	
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $^
	
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $^

print:
	@echo Directories: $(BINDIR) $(OBJDIR)

clean:
	$(RM) $(BINDIR)/* $(OBJDIR)/*.o

.PHONY: all clean uninstall

# -*- END:		RECIPES ------------------------------------------------------*-

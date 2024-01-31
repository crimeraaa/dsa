BIN_DIR = ./bin
OBJ_DIR = ./obj

C_EXE = $(BIN_DIR)/main
C_SRC = $(wildcard *.c)
C_OBJ = $(C_SRC:%.c=$(OBJ_DIR)/%.o)

CFLAGS += $(DEBUGFLAGS)

# Strangely, putting the order-only prerequisites here doesn't work properly.
all: $(C_EXE)

$(BIN_DIR) $(OBJ_DIR):
	$(MKDIR) $@

$(C_EXE): $(C_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $<
	
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<
	
clean:
	$(RM) $(C_EXE) $(C_OBJ)
	
.PHONY: all clean

ASM := nasm
ASM_FLAGS := -felf64 -gdwarf

link: compile
	clang -o test test.o string.o cstring.o io.o

compile: assemble
	clang -g -c -o test.o test.c

assemble:
	$(ASM) $(ASM_FLAGS) -o string.o string.asm
	$(ASM) $(ASM_FLAGS) -o cstring.o cstring.asm
	$(ASM) $(ASM_FLAGS) -o io.o io.asm

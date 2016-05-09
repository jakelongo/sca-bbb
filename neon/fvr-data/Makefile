SOURCES ?= socketNeon.c trigger.c
INSTRS  ?= eor.o add.o mul.o sub.o cmp.o

all: $(INSTRS)
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S 	 -o nodebug.out
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S -DDEBUG -o debug.out
	gcc -mfpu=neon socketNeon.c trigger.c execNeon.S -g	 -o GDBdebug.out

# Recipes for compiling pre-built assembly objs
%.o: neonExecute_INSTR.S
	gcc -mfpu=neon $< -D INSTR=veor -D WORD=u32 -c -o neonExecute_$@.o


.PHONY: all
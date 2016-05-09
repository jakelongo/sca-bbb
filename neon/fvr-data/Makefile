SOURCES ?= socketNeon.c trigger.c
OBJS    ?= eor.o add.o mul.o sub.o cmp.o


all: $(patsubst %.o,%,$(OBJS))
	gcc -mfpu=neon socketNeon.c trigger.c $(OBJS) -o nodebug.out
	gcc -mfpu=neon socketNeon.c trigger.c $(OBJS) -DDEBUG -o debug.out
	gcc -mfpu=neon socketNeon.c trigger.c $(OBJS) -g	 -o GDBdebug.out

# Recipes for compiling pre-built assembly objs
%: neonExecute_INSTR.S
	gcc -mfpu=neon $< -D INSTR=veor -D WORD=u32 -c -o neonExecute_$@


.PHONY: all
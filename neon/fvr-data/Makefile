SOURCES ?= socketNeon.c trigger.c
OBJS    ?= veor.o vadd.o vmul.o vsub.o vceq.o


all: $(patsubst %.o,%,$(OBJS))
	gcc -mfpu=neon socketNeon.c trigger.c $(OBJS)         -o nodebug.out
	gcc -mfpu=neon socketNeon.c trigger.c $(OBJS) -DDEBUG -o debug.out
	gcc -mfpu=neon socketNeon.c trigger.c $(OBJS) -g      -o GDBdebug.out

# Recipes for compiling pre-built assembly objs
%: neonExecute_INSTR.S
	gcc -mfpu=neon $< -D INSTR=$@ -D WORD=u32 -c -o $@.o

clean:
	rm *.out
	rm *.o

.PHONY: all makefile
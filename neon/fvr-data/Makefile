SOURCES ?= socketNeon.c trigger.c
OBJS    ?= veor.o vadd.o vmul.o vsub.o vceq.o


all: $(patsubst %.o,%,$(OBJS))
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS)         -o    nodebug.out
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS) -DDEBUG -o    debug.out
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS) -DDEBUG -g -o gdb_debug.out

# Recipes for compiling pre-built assembly objs
%: neonExecute_INSTR.S
	gcc -mfpu=neon $< -D INSTR=$@ -D WORD=u32 -c -o $@.o

clean:
	rm *.out
	rm *.o

.PHONY: all makefile
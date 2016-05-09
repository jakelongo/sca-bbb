SOURCES ?= socketNeon.c trigger.c
OBJS    ?= veor.o vadd.o vmul.o vsub.o vceq.o

.PHONY: all clean

all: $(OBJS)
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS)         -o    nodebug.out
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS) -DDEBUG -o    debug.out
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS) -DDEBUG -g -o gdb_debug.out

v%.o: neonExecute_INSTR.S
	gcc -mfpu=neon $< -D INSTR=$(basename $@) -D WORD=u32 -c -o $@

clean:
	rm -f *.out
	rm -f *.o
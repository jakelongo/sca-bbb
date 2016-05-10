SOURCES ?= socketNeon.c trigger.c

# Universal operations
OBJS_U   ?= $(addsuffix _u64.o , vand veor)

# Signed operations
OBJS_I8  ?= $(addsuffix _i8.o , vmul vadd vsub)
OBJS_I16 ?= $(addsuffix _i16.o, vmul vadd vsub)
OBJS_I32 ?= $(addsuffix _i32.o, vmul vadd vsub)
OBJS_I64 ?= $(addsuffix _i64.o,      vadd vsub)

OBJS ?= $(OBJS_U) $(OBJS_I8) $(OBJS_I16) $(OBJS_I32) $(OBJS_I64)


.PHONY: all clean

all: $(OBJS)
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS)         -o    nodebug.out
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS) -DDEBUG -o    debug.out
	gcc -mfpu=neon -std=c99 $(SOURCES) $(OBJS) -DDEBUG -g -o gdb_debug.out

# objects for i8
v%.o: neonExecute_INSTR.S
	gcc -mfpu=neon $< -D INSTR=$(basename $@) -D WORD=$(shell echo $@ | sed -e "s/.*_\(.*\)\.o/\1/g") -c -g -o $@

clean:
	rm -f *.out
	rm -f *.o
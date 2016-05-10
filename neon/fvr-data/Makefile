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

v%.o: neonExecute_INSTR.S
	$(eval INSTR := $(shell echo $@ | sed -e "s/\(.*\)_.*/\1/g"))
	$(eval WORD  := $(shell echo $@ | sed -e "s/.*_\(.*\)\.o/\1/g"))
	echo "void neonExecute_$(INSTR)$(WORD)(uint8_t** membank);" >> auto_neon_header.h
	echo gcc -mfpu=neon $< -D INSTR=$(INSTR) -D WORD=$(WORD) -c -g -o $@

clean:
	rm -f *.out
	rm -f *.o
	rm -f auto_neon_header.h
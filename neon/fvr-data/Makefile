SRCDIR  := src
SRCS    := $(wildcard $(SRCDIR)/*.c)

# Universal operations
OBJS  := $(addsuffix _u64.o , vand veor)

# Signed operations
OBJS  += $(addsuffix _i8.o , vmul vadd vsub)
OBJS  += $(addsuffix _i16.o, vmul vadd vsub)
OBJS  += $(addsuffix _i32.o, vmul vadd vsub)
OBJS  += $(addsuffix _i64.o,      vadd vsub)

.PHONY: all clean
CFLAGS   := -mfpu=neon -std=c99
DBGFLAGS := -DDEBUG
GDBFLAGS := $(DBGFLAGS) -g

ASFLAGS  := -mfpu=neon

all: $(OBJS)
	$(CC) $(CFLAGS) $(SRCS) -I$(SRCDIR) $(OBJS)             -o nodebug.out
	$(CC) $(CFLAGS) $(SRCS) -I$(SRCDIR) $(OBJS) $(DBGFLAGS) -o debug.out
	$(CC) $(CFLAGS) $(SRCS) -I$(SRCDIR) $(OBJS) $(GDBFLAGS) -o gdb_debug.out

v%.o: $(SRCDIR)/neonExecute_INSTR.S
	$(eval INSTR := $(shell echo $@ | sed -e "s/\(.*\)_.*/\1/g"))
	$(eval WORD  := $(shell echo $@ | sed -e "s/.*_\(.*\)\.o/\1/g"))
	$(CC) $(ASFLAGS) $< -D INSTR=$(INSTR) -D WORD=$(WORD) -c -o $@

clean:
	rm -f *.out
	rm -f *.o
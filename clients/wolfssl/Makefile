OBJS   := $(patsubst %.c,%.o,$(wildcard *.c))
OBJS   += $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJS   += $(patsubst %.S,%.o,$(wildcard *.S))

.PHONY: all clean

CFLAGS   := -std=c99
CPPFLAGS :=

ASFLAGS  :=

LDFLAGS  := -L/usr/local/lib
LDLIBS   := -lwolfssl

DBGFLAGS := -g

CC  ?= gcc
CXX ?= g++

all: wolfssl-client

wolfssl-client: $(OBJS)
	$(CC) $(LDFLAGS) $(LDLIBS) -o $@ $^

%.o: %.S
	$(CC)  -o $@ $(CFLAGS) $(ASFLAGS) -c $^

%.o: %.c
	$(CC)  -o $@ $(CFLAGS)   -c $^

%.o: %.cpp
	$(CXX) -o $@ $(CPPFLAGS) -c $^

clean:
	rm -f $(OBJS)

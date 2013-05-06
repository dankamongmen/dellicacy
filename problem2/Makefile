.DELETE_ON_ERROR:
.PHONY: all test clean

BIN:=ipc
CFLAGS?=-O2 -Wall -Werror
LFLAGS?=-Wl,-O1
SRC:=$(wildcard *.c)
OBJS:=$(addsuffix .o,$(basename $(SRC)))
INCS:=$(wildcard *.h)

all: $(BIN)
	
$(BIN):	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

%.o: %.c $(INCS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJS) $(BIN)

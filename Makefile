CC=gcc
CFLAGS := -std=c99 -pedantic -Wall
CFLAGS += -O2

TARGETS=bitgram

all: $(TARGETS)

.c.s:
	$(CC) $(CFLAGS) -S -o $@ $<

clean:
	rm -rf $(TARGETS) *.o

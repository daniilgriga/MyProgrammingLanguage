SOURCES=main.c log.c syntax.c tokens.c tree.c buffer.c
EXE=lang

CC=gcc
CFLAGS=-Wall -g -lm -fsanitize=address

OBJS=$(SOURCES:.c=.o)

.PHONY: all clean

all: $(EXE)


clean:
	rm -rf *.o $(EXE)

$(EXE): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $@

.c.o:
	@$(CC) -I ../include -c $(CFLAGS) $< -o $@

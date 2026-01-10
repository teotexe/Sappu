CC = gcc
CFLAGS = -Wall -Wextra
SRCDIR = src

SRCS = gates.c memory.c alu.c control.c computer.c test.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: computer assembler

assembler: assembler.c
	$(CC) $(CFLAGS) $< -o $@

computer: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@
	rm -f $(OBJS)

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) computer


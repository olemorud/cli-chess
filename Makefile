
CC = clang
CFLAGS = -Ofast -fsanitize=address -static-libsan -ggdb3 -Wall -Wextra -Werror

.PHONY: all run gdb clean

all: bin/chess

clean:
	rm bin/*

gdb: bin/chess
	gdb $<

run: bin/chess
	./$<

bin/chess: chess.c bin
	$(CC) $(CFLAGS) $< -o $@


bin:
	mkdir -p bin


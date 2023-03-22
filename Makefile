
CC = clang
LD = ld.lld
CFLAGS =  -Iinclude
CFLAGS += -Ofast -ggdb3
CFLAGS += -Wall -Wextra -Werror
CFLAGS += -fsanitize=address
LDFLAGS = -fuse-ld=lld
LDFLAGS = -fsanitize=address

_OBJ = chess.o common.o graphics.o pieces.o util.o
OBJ = $(addprefix obj/, $(_OBJ))

all: bin/chess

obj:
	mkdir -p $@

bin:
	mkdir -p $@

docs:
	doxygen doxygen-config

clean:
	rm bin/* obj/*.o

gdb: bin/chess
	gdb $<

run: bin/chess
	./$<

obj/%.o: src/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

bin/chess: $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

.PHONY: all run gdb clean docs


CC = gcc
#CFLAGS += -O0 -ggdb3
CFLAGS += -Ofast -ggdb3
CFLAGS += -Wall -Wextra -Wno-unused-function -rdynamic
#CFLAGS += -fsanitize=address
LDFLAGS = -fuse-ld=lld -rdynamic
#LDFLAGS += -fsanitize=address

_OBJ = chess.o
OBJ = $(addprefix obj/, $(_OBJ))

TEST_DIR = testing

all: bin/chess

test: $(TEST_DIR)/bin/test_threatmap

obj:
	mkdir -p $@

bin:
	mkdir -p $@

$(TEST_DIR):
	mkdir -p $@/bin

clean:
	rm bin/* obj/*.o

obj/%.o: src/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

bin/chess: $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

$(TEST_DIR)/bin/test_%: testing/test_%.c obj/%.o obj/util.o
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^

.PHONY: all clean docs test

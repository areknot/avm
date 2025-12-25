CC = gcc
INCLUDES = -I./third_party/tree-sitter/include \
           -I./third_party/tree-sitter/src
CFLAGS = -std=gnu11 -Wall -Wextra $(INCLUDES)

SRCS = $(wildcard ./src/*.c)              \
       ./src/tree-sitter-avm/src/parser.c \
       ./third_party/tree-sitter/src/lib.c
OBJS = $(SRCS:.c=.o)
CORE_OBJS = $(filter-out ./src/main.o,$(OBJS))
TARGET = avm

TEST_SRCS = $(wildcard ./tests/*.c)
TEST_OBJS = $(TEST_SRCS:.c=.o)
TEST_TARGET = interp_tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(TEST_TARGET): $(CORE_OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

tests/%.o: tests/%.c
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

avm-echo: $(CORE_OBJS) ./tests/avm-echo/main.c
	$(CC) $(CFLAGS) -I./src $^ -o avm-echo

tree-sitter-avm: src/tree-sitter-avm/grammar.js
	cd src/tree-sitter-avm; tree-sitter generate

./third_party/tree-sitter/src/lib.o: ./third_party/tree-sitter/src/lib.c
	$(CC) $(CFLAGS) -c $^ -o $@

./src/tree-sitter-avm/src/parser.o: ./src/tree-sitter-avm/src/parser.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(OBJS) $(TEST_OBJS) avm-echo

.PHONY: all clean

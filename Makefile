CC = gcc
CFLAGS = -std=c11 -Wall -Wextra

SRCS = $(wildcard ./src/*.c)
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

clean:
	rm -f $(TARGET) $(TEST_TARGET) $(OBJS) $(TEST_OBJS)

.PHONY: all clean

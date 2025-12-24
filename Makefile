CC = gcc
CFLAGS = -std=c11 -Wall -Wextra

SRCS = $(wildcard ./src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = avm

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean

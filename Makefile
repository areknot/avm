
CC = gcc -std=c11

all: avm

avm: src/main.c
	$(CC) $^ -o $@

clean:
	rm ./avm

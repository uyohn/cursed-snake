run: all
	./main

all:
	gcc main.c -o main -lncurses -g

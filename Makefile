all: main.c
	cc -I./uthash/src -Lharu -g main.c -o main

all: main.c
	cc -I./uthash/src -I/usr/local/include/ -DHPDF_SHARED -lhpdf -lpng -lz -lm -g main.c -o main

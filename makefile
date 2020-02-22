all: phsp

phsp: phsp.c
	gcc phsp.c -o output -lm -pthread


CC=gcc

default: all

all: exampleregex

exampleregex: example.c
	$(CC) -o exampleregex ./example.c
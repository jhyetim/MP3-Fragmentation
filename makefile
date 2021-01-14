CC = gcc

CFLAGS += -std=gnu11
CFLAGS += -Wall -Werror

all: defrag.c
	${CC} ${CFLAGS} -ggdb3 -o defrag defrag.c -pthread 


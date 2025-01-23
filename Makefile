CC=cc
OPT=-O3
CFLAGS=-Wall -Wextra -Wno-unused-label -ggdb -std=c11 $(OPT)
INCLUDES=-I./include
LIBS=-L./lib/ -lraylib -lm

all: game librayapi.a

game:
	$(CC) $(CFLAGS) $(INCLUDES) -o game ./src/game.c -lrayapi $(LIBS)

librayapi.a:
	$(CC) $(CFLAGS) $(INCLUDES) -c include/rayapi.c -o rayapi.o $(LIBS)
	ar rcs lib/librayapi.a rayapi.o
	rm rayapi.o

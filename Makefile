CC=gcc
OPT=-O3
EXTRA_INDENT=$(if $(EXTRA), " ")
CFLAGS=-Wall -Wextra -pedantic -Wno-unused-label -ggdb -std=c11 $(OPT)$(EXTRA_INDENT)$(EXTRA)
INCLUDES=-I./include
LIBS=-L./lib/ -lraylib -lm

all: game examples

game:
	$(CC) $(CFLAGS) $(INCLUDES) -o game ./src/game.c $(LIBS)

examples: examples/split examples/vars examples/objects

examples/split: examples/split.c include/rayapi.h
	$(CC) $(CFLAGS) $(INCLUDES) -o examples/split examples/split.c $(LIBS)

examples/vars: examples/vars.c include/rayapi.h
	$(CC) $(CFLAGS) $(INCLUDES) -o examples/vars examples/vars.c -D__SRC_LOC__="\"./examples/\"" $(LIBS)

examples/basic_shapes: examples/basic_shapes.c include/rayapi.h
	$(CC) $(CFLAGS) $(INCLUDES) -o examples/basic_shapes examples/basic_shapes.c $(LIBS)

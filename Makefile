CC = clang
CFLAGS = -Wall -Werror -g
LIBS = -lSDL3

default: main

main: main.c chip8.c
	$(CC) $(CFLAGS) main.c chip8.c $(LIBS) -o main

clean:
	rm main

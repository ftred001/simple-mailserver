CC = gcc
CFLAGS = -g -ansi -pedantic -Wall
	
main: main.c database.c
	$(CC) $(CFLAGS) -o main main.c database.c 

run: main
	./main
	
clean:
	rm main

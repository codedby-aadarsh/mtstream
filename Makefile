CC = gcc
CFLAGS = -Wall -g
all : mtstream.c 
	$(CC) $(CFLAGS) mtstream.c -o mtstream -lpthread

clean:
	rm -f mtstream


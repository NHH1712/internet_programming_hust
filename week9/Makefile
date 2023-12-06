all: server client client1 client2

server: server.c
	gcc -o server server.c

client: client.c
	gcc -o client client.c

client1: client1.c
	gcc -o client1 client1.c

client2: client2.c
	gcc -o client2 client2.c
clean:
	rm -f server client1 client2
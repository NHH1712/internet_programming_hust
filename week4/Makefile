all: udp_server udp_client

udp_server: udp_server.c
	gcc -o udp_server udp_server.c

udp_client: udp_client.c
	gcc -o udp_client udp_client.c

clean:
	rm -f udp_server udp_client
all:
	gcc -o mainClient mainClient.c funcClient.c funcServer.c model/connection.c
	gcc -o mainServer mainServer.c funcClient.c funcServer.c model/connection.c
client:
	./mainClient
server:
	./mainServer

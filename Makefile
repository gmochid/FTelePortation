all:
	gcc -o main main.c funcClient.c funcServer.c model/connection.c
client:
	./main
server:
	./main --server
clean:
	rm main
	

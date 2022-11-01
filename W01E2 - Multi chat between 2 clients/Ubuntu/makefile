server: cleansv
	gcc -pthread server.c -o server
	./server
client: cleancl
	gcc -pthread client.c -o client
	./client
cleansv:
	rm -rf server
cleancl:
	rm -rf client

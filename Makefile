all: snc

snc: snc.o
	g++ -pthread snc.o -o snc

snc.o: snc.c
	g++ -pthread -c -g snc.c 

clean: 
	rm -rf *.o

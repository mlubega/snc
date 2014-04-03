all: snc

snc: snc.o
	g++ snc.o -o snc

snc.o: snc.c
	g++ -c snc.c

clean: 
	rm -rf *.o

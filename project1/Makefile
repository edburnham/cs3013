all: mc0 mc1 mc2
 
mc0: mc0.o
	gcc -Wall mc0.o -o mc0

mc0.o: mc0.c
	gcc -Wall -c mc0.c

mc1: mc1.o
	gcc -Wall mc1.o -o mc1

mc1.o: mc1.c
	gcc -Wall -c mc1.c

mc2: mc2.o
	gcc -Wall mc2.o -o mc2
	
mc2.o: mc2.c
	gcc -Wall -c mc2.c
	
clean:
	rm -f mc0.o mc0 
	rm -f mc1.o mc1
	rm -f mc2.o mc2

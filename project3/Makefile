all: mac
 
mac: mac.o
	gcc -Wall mac.o -o mac

mac.o: mac.c
	gcc -Wall -c mac.c
	
clean:
	rm -f mac.o mac 
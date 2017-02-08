all: mac
 
mac: mac.o
	gcc -Wall -pthread mac.o -o mac

mac.o: mac.c
	gcc -Wall -pthread -c mac.c
	
clean:
	rm -f mac.o mac 
all: macsem macmutex
 
macsem: macsem.o
	gcc -Wall -pthread macsem.o -o macsem

macmutex: macmutex.o
	gcc -Wall -pthread macmutex.o -o macmutex
	
macsem.o: macsem.c
	gcc -Wall -pthread -c macsem.c
	
macmutex.o: macmutex.c
	gcc -Wall -pthread -c macmutex.c
	
clean:
	rm -f macsem.o macsem 
	rm -f macmutex.o macmutex
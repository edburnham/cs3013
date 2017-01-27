all: mckasperfee

mckasperfee: mckasperfee.o
	gcc -Wall mckasperfee.o -o mckasperfee

mckasperfee.o: mckasperfee.c
	gcc -Wall -c mckasperfee.c

clean:
	rm -f mckasperfee.o mckasperfee 

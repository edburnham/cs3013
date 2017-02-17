all: vmem

vmem: vmem.o
	gcc -Wall vmem.o -o vmem

vmem.o: vmem.c
	gcc -Wall -c vmem.c

clean:
	rm -f vmem.o vmem

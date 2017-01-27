obj-m := mckasperfee.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	
testcalls: testcalls.o
	gcc -Wall testcalls.o -o testcalls.c
	
testcalls.o: testcalls.c
	gcc -Wall -c testcalls..c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

// These values MUST match the unistd_32.h modifications:
#define __NR_cs3013_syscall1 377
#define __NR_cs3013_syscall2 378
#define __NR_cs3013_syscall3 379

struct ancestry{
    pid_t ancestors[10];
    pid_t siblings[100];
    pid_t children[100];
};

struct ancestry buffu;

unsigned short uPid;

long testCall1 (void) {
    return (long) syscall(__NR_cs3013_syscall1);
}

long testCall2 (void) {
    return (long) syscall(__NR_cs3013_syscall2, &uPid, &buffu);
}

long testCall3 (void) {
    return (long) syscall(__NR_cs3013_syscall3);
}

int main (int argc, char* argv[]) {
    unsigned short input;

    if(argc == 2){
	    input = (unsigned short)atoi(argv[1]);
	    uPid = input;
        
	    testCall2();
        
        if(uPid == 0){
            printf("PID %d does not esist.\n", input);
            return 0;
        }else{
            puts("Printed ancestry tree to syslog");   
        }
    }
    else if(argc == 1){
        puts("Need to input PID");
        return 0;
    }
    else {
	    puts("Usage: procAncestry <PID>");
	    return 0;
    }  
}

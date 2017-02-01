#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

// These values MUST match the unistd_32.h modifications:
#define __NR_cs3013_syscall1 377
#define __NR_cs3013_syscall2 378
#define __NR_cs3013_syscall3 379

struct ancestry{
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

long testCall2 (unsigned short *target_pid, struct ancestry *response) {
    return (long) syscall(__NR_cs3013_syscall2, &target_pid, response);
}

int main (int argc, char* argv[]) {
	struct ancestry *response = NULL;

	if(argc == 2){
		testCall2((unsigned short *)atoi(argv[1]), response);
		return 0;
	}else{
		puts("Exiting: need to pass only 1 agument: <PID>");
		return 0;
	}
	return 0;  
}
#include <sys/syscall.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
//#include <linux/include/asm/current.h>//current macro
//#include <asm/unistd.h>

// These values MUST match the unistd_32.h modifications:
#define __NR_cs3013_syscall1 377
#define __NR_cs3013_syscall2 378
#define __NR_cs3013_syscall3 379

//asmlinkage long (*ref_sys_cs3013_syscall2)(unsigned short *target_pid, struct ancestry *response);


struct ancestry{
	pid_t ancestors[10];
	pid_t siblings[100];
	pid_t children[100];
};

long testCall2 (void) {
    return (long) syscall(__NR_cs3013_syscall2);
}

//int argc, char** argv
int main () {
	
    //printf("\tcs3013_syscall2: %ld\n", testCall2());
	testCall2();
	
    return 0;
}

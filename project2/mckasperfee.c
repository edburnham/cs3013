#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/highmem.h>
#include <linux/pid.h>
#include <asm/unistd.h>
#include <asm/errno.h>

struct ancestry {
    pid_t ancestors[10];
    pid_t siblings[100];
    pid_t children[100];
};

struct ancestry buffy;

/*to keep track of the number of PIDS for each member, for printing*/
int numChild = 0; 
int numSib = 0;
int numAns = 0;

unsigned short yPid;
unsigned short zero;//user space flag to send back for non existence of pid

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall2)(void);

void traverse_parent(struct task_struct *task){
    struct task_struct * temp_parent;
    int i = 0;
    numAns = 0;
    
    temp_parent = task->parent;
    
    while(temp_parent->pid != 0){
        buffy.ancestors[i] = temp_parent->pid;
        temp_parent = temp_parent->parent;
        i++;
    }    
    numAns = i;
}

void traverse_children(struct task_struct *task){
    struct task_struct *child;
    int i = 0;
    numChild = 0;
    
    list_for_each_entry(child, &(task->children), sibling){
	buffy.children[i] = child->pid;
	i++;
    }    
    numChild = i;
}

void traverse_sibling(struct task_struct *task){
    struct task_struct *list;
    struct task_struct *task_parent;
    int i = 0;
    int ppid = 0;
    numSib = 0;
    
    task_parent = task->parent;
    ppid = task_parent->pid;
    
    if(ppid > 0) {
	list_for_each_entry(list, &(task_parent->children), sibling){
	    buffy.siblings[i] = list->pid;
	    i++;
	}
	numSib = i;
    }
    else {numSib = 0;}
}

void printAnsTree(void){
    int j, k, l;
    
    printk("-------- PID %d Ancestry Tree -------\n", yPid);
    printk("------------- Ancestors -------------\n");
    for(j = 0; j < numAns; j++){
        printk("Ancestor %d: %d\n", j + 1, buffy.ancestors[j]);
    }
    printk("------------- Children -------------\n");
    for(k = 0; k < numChild; k++){
        printk("Child %d: %d\n", k + 1, buffy.children[k]);
    }
    printk("------------- Siblings -------------\n");
    for(l = 0; l < numSib; l++){
        printk("Sibling %d: %d\n", l + 1, buffy.siblings[l]);
    }
}

asmlinkage long new_sys_cs3013_syscall2(unsigned short *target_pid, struct ancestry *response) {
    struct task_struct *tempTask;
   
    if(copy_from_user(&buffy, response, sizeof(buffy))){     
	return EFAULT;
    }
    if(copy_from_user(&yPid, target_pid, sizeof(yPid))) {
	return EFAULT;
    }
    
    tempTask = pid_task(find_vpid(yPid), PIDTYPE_PID);

    if (tempTask == NULL) {
        printk("PID %d does not exist.\n", yPid);
        zero = 0;
        copy_to_user(target_pid, &zero, sizeof(short));
        return 0;
    }
    else {
        traverse_parent(tempTask);
        traverse_children(tempTask);
        traverse_sibling(tempTask);
        printAnsTree();
	copy_to_user(response, &buffy, sizeof(buffy));
    }
    return 0;
}

static unsigned long **find_sys_call_table(void) {
    unsigned long int offset = PAGE_OFFSET;
    unsigned long **sct;
    while (offset < ULLONG_MAX) {
	sct = (unsigned long **)offset;
	if (sct[__NR_close] == (unsigned long *) sys_close) {
	    printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX\n",
		   (unsigned long) sct);
	    return sct;
	}
	offset += sizeof(void *);
    }
    return NULL;
}

static void disable_page_protection(void) {
    /*
      Control Register 0 (cr0) governs how the CPU operates.
      Bit #16, if set, prevents the CPU from writing to memory marked as
      read only. Well, our system call table meets that description.
      But, we can simply turn off this bit in cr0 to allow us to make
      changes. We read in the current value of the register (32 or 64
      bits wide), and AND that with a value where all bits are 0 except
      the 16th bit (using a negation operation), causing the write_cr0
      value to have the 16th bit cleared (with all other bits staying
      the same. We will thus be able to write to the protected memory.
      It’s good to be the kernel!
    */
    write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
    /*
      See the above description for cr0. Here, we use an OR to set the
      16th bit to re-enable write protection on the CPU.
    */
    write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
    /* Find the system call table */
    if(!(sys_call_table = find_sys_call_table())) {
	/* Well, that didn’t work.
	   Cancel the module loading step. */
	return -1;
    }
    
    /* Store a copy of all the existing functions */
    ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
    
    /* Replace the existing system calls */
    disable_page_protection();
    sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;
    enable_page_protection();
    /* And indicate the load was successful */
    printk(KERN_INFO "Loaded interceptor!\n");
    return 0;
}

static void __exit interceptor_end(void) {
    /* If we don’t know what the syscall table is, don’t bother. */
    if(!sys_call_table)
	return;
    /* Revert all system calls to what they were before we began. */
    disable_page_protection();
    sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
    enable_page_protection();
    printk(KERN_INFO "Unloaded interceptor!\n");
}
MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);

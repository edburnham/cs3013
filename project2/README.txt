Edward Burnham
Michael Caldwell
CS3013 C Term 2017
Project 2

Phase 1:

*** Must be in Phase 1 folder ***
Compile using: make all (compiles LKM)
	       make test (compliles testcalls.c, the test file)
	    OR sudo ./install.sh 
	       install.sh executes make clean, all, test, rmmod, and insmod  

Run test file test.txt with "VIRUS" inside via the testcalls program: ./testcalls < test.txt
Open syslog: tail /var/log/syslog
	       
In phase 1 system calls sys_open, sys_read, and sys_close were intercepted. A copy of these existing system cals were saved and replaced with the new version of these calls. If the uid is 1000 or greater, printk() a message to the syslog and return a pointer to the (original) system call copy. The system call table is he restored towith the original system calls.




Phase 2:

*** Must be in Phase 2 folder ***
Compile using: make all (compiles LKM)
	       make pans (compliles procAncestry.c, the user-land file)
	    OR sudo ./install.sh 
	       install.sh executes make clean, all, test, rmmod, and insmod 
	       
	       test.sh: 
	       		runs procAncestry with various PIDS making sure correct errors print to stdout
	       		
	       test_procAncestry.c: 
	       		creates over 100 sibling processes to make sure the kernel side would not overfill the ancestry sibling array, verified in syslog
	       		printed user side ancestry tree to confirm proper passing from kernel side

Run user-land program: ./procAncestry <PID> < test.txt
Open syslog: tail /var/log/syslog to see the ancestry	
     
In phase 2, syscall2 was intercepted was intercepted to discover the geneaology of a particular process. The process ID is passed to procAncestry.c and the ID and ancestry structure is passed the kernel implementaion of syscall2. Every process has a task struct. This task struct was located by using the function pid_task(). After the task_struct for this process was obtained, it was traversed to find the process's, ancestors, siblings and children using list_for_each_entry(). The ancesty struct is filled during traversal, then the results are printed to syslog by with printk via the printAnsTree function. The struct is passed to the user space as well. If the user enters in a PID that doesn't exist, they are informed so. The PID is searched for via mckasperfee.c and if the PID doesn't exist a flag is sent to user land and he user is informed that the PID soesn't exist.


/*
Author:				Ed Burnham, Mike Caldwell
Date:				1/16/2017 - 1/24/2017
Version:			2
Project ID:			Project 1
CS Class:			CS 3013
Programming Language:		C 
OS/Hardware dependencies:	Uses Linux system calls

Problem Description:		Midday Commander - text based system interaction

Overall Design:			
	System structure	The program has a flat structure with a main function and a few small functions called from main
	Data representation	most data is represented in primitive variable types. there is a struct to contain the information for bg processes
	Algorithms 		The program makes no use of any algorithms worth mentioning

Program Assumptions 
      and Restrictions:		There are limits on user input, details available in README

Interfaces:			system calls, stdin, stderr, stdout

Implementation Details:
	Data			Struct named bgProcesses contains information for background processes	        
	Functions	        Use of fork(), exec() and wait() functions for process control

How to build the program:	make all

Program Source:	
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <sys/wait.h>

#define ARG_LIMIT 32 // limit on number of args
#define ARG_LENGTH 128 // limit on length of an arg

int numberOfCommands = -1;
int lastTimeMaj = 0;
int lastTimeMin = 0;
int status = 0;
int bgpIndex = 0;
int maxReached = 0;

double wall_milli = 0; // wall clock timer

struct timeval start, stop;
struct rusage usage;
struct bgProcess { // struct to describe background processes
    int bgid;
    int pid;
    char *command;
    int running;
};

struct bgProcess bgProcesses[ARG_LIMIT];

char commandsToPrint[ARG_LIMIT][ARG_LENGTH];//user added commands
int commandIDs[ARG_LIMIT];//command IDs corresponding to the user added commands

void addCommandOrPrint(char *command, int addOrPrint) {//0 passed to addOrPrint just prints the user added commands
    int i, j, k;                                       //1 passed to addOrPrint only adds the command to the list

    if(maxReached) {
	k = 7; //maximum number of user added commands
    }
    else {
	k = (numberOfCommands + 1); // otherwise k is the number of commands plus 1
    }
      
    if (addOrPrint) {//add command
      strncpy(commandsToPrint[numberOfCommands], command, 128);//copy the user added command to the list
      commandIDs[numberOfCommands] = numberOfCommands + 3;  // Offset of 3 between number of commands and list position

    } else {//don't add command just print them with IDs
        for (i = 0; i < k; i++) {
            printf("   %d. ", commandIDs[i]);
            for (j = 0; commandsToPrint[i][j] != '\0'; j++) {
                printf("%c", commandsToPrint[i][j]);
            }
            puts("      : User added command");
        }
    }
}

void printOptions() {//print menu function
    puts("G'day, Commander! What command would would you like to run?");
    puts("   0. whoami   : Prints out the result of the whoami command");
    puts("   1. last     : Prints out the results of the last command");
    puts("   2. ls       : Prints out the result of a listing on a user specified path");
    addCommandOrPrint(" ", 0); // call the add command or print command function
    puts("   a. add command : Adds a new command to the menu.");
    puts("   c. change directory : Changes process working directory ");
    puts("   e. exit : Leave Mid-Day Commander");
    puts("   p. pwd : Prints working directory ");
    puts("   r. running processes : Print list of running processes");
    printf("Option?: ");
}

void printStats() {//print statistics function
    wall_milli = 0; // reset wall clock to 0
    puts("");
    puts("-- Statistics --");
    wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec) / 1000000.0); // calculating wall time
    printf("Elapsed time: %f milliseconds\n", 1000.0 * wall_milli);
    getrusage(RUSAGE_CHILDREN, &usage); // get rusage from children
    printf("Page Faults: %ld\n", (usage.ru_majflt - lastTimeMaj)); // subtracting from cumulative total
    printf("Page Faults (reclaimed): %ld\n", (usage.ru_minflt - lastTimeMin)); // this results in the value for the process of interest
    puts("");
    lastTimeMaj = usage.ru_majflt;//time to subtract from cumulative time held in struct usage
    lastTimeMin = usage.ru_minflt;
}

void processExit(int waitPid) {
    int bgIndex, i;

    if (waitPid != 0) { // this check is for the exit condition wait4, so when it calls this and its 0, we dont do the rest
	
	gettimeofday(&stop, NULL); // stop time for wall clock timer

    	for (i = 0; i < 32; i++) { // get index of process by matching pid from wait
	    if (bgProcesses[i].pid == waitPid) {
		bgIndex = i;
	    }
	}
	
	// Print out completion notification
	printf("-- Job Complete [%d] --\n", bgProcesses[bgIndex].bgid);
	printf("Process ID: %d\n", bgProcesses[bgIndex].pid);
	bgProcesses[bgIndex].running = 0; // toggle running flag
	printStats();
    }
}

int checkLength(char* command){
    int count = 0;
    int i;
	
    if(strlen(command) >= 128){
	return 0;
    }	
    for (i = 0; i < strlen(command) + 1; i++) {
	if (command[i] == ' ') {
	    count++;
	}
    }
    if(count > 32){
     	return 0;
    }
    else{
    	return 1;
    }
}

void userCommandExec(int commandIndex) {//executes user added command
    int i, waitPid;
    int tokens = 0;
    int tempIndex = 0;

    char tempCommand[ARG_LIMIT + 1][ARG_LENGTH];//temps for parsing and copying
    char tempCommand2[ARG_LIMIT + 1][ARG_LENGTH];
    char *args[ARG_LIMIT + 1] = {NULL};//execvp needs a array of char*

    memset(tempCommand, 0, sizeof(tempCommand[0][0]) * (ARG_LIMIT + 1) * ARG_LENGTH); // Initializing temporary arrays
    memset(tempCommand2, 0, sizeof(tempCommand2[0][0]) * (ARG_LIMIT + 1) * ARG_LENGTH);
	
    strcpy(tempCommand[0], commandsToPrint[commandIndex]);//make local copy of command to execute, accessed by command Index
    tempCommand[0][ARG_LENGTH - 1] = '\0'; 

    /*parse command into arg and command strings*/
    for (i = 0; i < strlen(tempCommand[0]) + 1; i++) {
        if ((tempCommand[0][i] == ' ') || (tempCommand[0][i] == '\0')) {
            args[tokens] = strdup(tempCommand2[tokens]);//parsed command holds command and args
            tempIndex = i + 1;//kepping track of i so the next array fills from element zero
            tokens++;//move to the next string in array if strings
        } else {
            tempCommand2[tokens][i - tempIndex] = tempCommand[0][i];//copy character by character
        }
    }
    
    if (!strncmp(args[tokens - 1], "&", 1)) { // if it's a background process, denoted by &
        args[tokens - 1] = NULL; // stripping off '\n' char
        
	bgProcesses[bgpIndex].pid = fork(); // forking, storing PID in ids
        bgProcesses[bgpIndex].bgid = bgpIndex; // storing the bgid
        bgProcesses[bgpIndex].command = commandsToPrint[commandIndex]; // command string
	bgProcesses[bgpIndex].running = 1; // running flag
	
	gettimeofday(&start, NULL); //getting time
	
        if (bgProcesses[bgpIndex].pid == 0) { // if CHILD
            execvp(args[0], args); // executing command in background
        } else { // in PARENT
            printf("\n-- Command: %s --\n", commandsToPrint[commandIndex]);
            printf("[%d] %d\n\n", bgpIndex, bgProcesses[bgpIndex].pid);
            while (1) {
                waitPid = wait3(&status, WNOHANG, &usage); // checking to see if bg process finished right away, storing pid so we can pass it
                if (waitPid > 0) { // if it has finished
		    processExit(waitPid); // exit gracefully
		} else {
                    break;
                }
            }
	}
	
	bgpIndex++; // index for tracking total background processes
	
    } else { // command is NOT a background process
        gettimeofday(&start, NULL); //getting time
        if (fork() == 0) { // the child
	    execvp(args[0], args); 
        } else { // le parent
            printf("\n-- Command: %s --\n", commandsToPrint[commandIndex]);            
            while (1) {
                if (wait3(&status, 0, &usage) > 0) { // waiting to see if a processed finished, with hang
		    gettimeofday(&stop, NULL); 
		    printStats();
                }
		else {
		    break;
		}
            }
        }
    }
}

int main(int argc, char *argv[]) {
    char input[128]; // input buffer
    char inpath[128]; // path input buffer for ls command
    char inarg[128]; // arguement input buffer for ls command
    char dirChange[128]; // input buffer for chdir command
    char commandToAdd[128]; // string of the user added command
    char *check_EOF; // checking EOF when files are passed as input
    int id = 3; // id for tracking user added command menu placement locally
    int waitPid = 0; // PID returned from wait3 command
    int addedCommandExists = 0; // flag for checking if a command has been added already
    int i;
    
    puts("===== Mid-Day Commander, v2 =====");

    while (1) {

        waitPid = wait3(&status, WNOHANG, &usage); // checking for any recently finish bg processes
        if (waitPid > 0) {
	    processExit(waitPid); // exiting gracefully
        } else {
            printOptions(); 
	    check_EOF = fgets(input, sizeof(input), stdin); // get user input from stdin
	    input[strlen(input) - 1] = '\0'; // stip newline char
	    if (strlen(input) > 1) {//if the argument is greater than 2 long, then it doesn't exist
		fprintf(stderr, "\nYou have entered an incorrect option.\n\n");
	    }
	    else if (!strncmp(input, "0", 1)) {//option 0
                puts("");
                puts("-- Who Am I? --");
                gettimeofday(&start, NULL);
                system("whoami"); // system call to fork and exec whoami
                gettimeofday(&stop, NULL);
                printStats();

            } else if (!strncmp(input, "1", 1)) {//option 1
                puts("");
                puts("-- Last Logins --");
                gettimeofday(&start, NULL);
                system("last"); // system call to fork and exec last
                gettimeofday(&stop, NULL);
                printStats();
		
            } else if (!strncmp(input, "2", 1)) {//option 2
                puts("");
                puts("-- Directory Listing --");
                printf("Arguments?: ");
                fgets(inarg, sizeof(inarg), stdin);
                printf("Path?: ");
                fgets(inpath, sizeof(inpath), stdin);
				
                if ((inarg[0] == '\n') && (inpath[0] != '\n')) { // handling special cases like no path or no arg input for ls
                    inpath[strlen(inpath) - 1] = '\0';
                    gettimeofday(&start, NULL);
                    if (fork() == 0) {
                        puts("");
                        execl("/bin/ls", "ls", inpath, (char *) NULL);
                    } else {
                        wait(NULL);
                        gettimeofday(&stop, NULL);
                        printStats();
                    }
                } else if ((inarg[0] != '\n') && (inpath[0] == '\n')) {
                    inarg[strlen(inarg) - 1] = '\0';
                    gettimeofday(&start, NULL);
                    if (fork() == 0) {
                        puts("");
                        execl("/bin/ls", "ls", inarg, (char *) NULL);
                    } else {
                        wait(NULL);
                        gettimeofday(&stop, NULL);
                        printStats();
                    }
                } else if (inarg[0] == '\n' && inpath[0] == '\n') {
                    gettimeofday(&start, NULL);
                    if (fork() == 0) {
                        puts("");
                        execl("/bin/ls", "ls", (char *) NULL);
                    } else {
                        wait(NULL);
                        gettimeofday(&stop, NULL);
                        printStats();
                    }
                } else { // last of the special cases for ls command input
                    inarg[strlen(inarg) - 1] = '\0';
                    inpath[strlen(inpath) - 1] = '\0';

                    gettimeofday(&start, NULL);
                    if (fork() == 0) {
                        puts("");
                        execl("/bin/ls", "ls", inarg, inpath, (char *) NULL);
                    } else {
                        wait(NULL);
                        gettimeofday(&stop, NULL);
                        printStats();
                    }
                }
            } else if (!strncmp(input, "a", 1)) {//option a
		if (id == 10) { // checking if the maximum user added command limit has been reached
		    puts("You've reached the maximum number of user commands. New commands overwrite old ones. ");
		    id = 3; // reseting local tracking variables
		    numberOfCommands = -1;
		    maxReached = 1; // flag for maximum reached
		}
		
		while(1){
			puts("");
			puts("-- Add a command --");
		        printf("Command to add?: ");
		        fgets(commandToAdd, sizeof(commandToAdd), stdin); // get command to add from user
		        commandToAdd[strlen(commandToAdd) - 1] = '\0'; // stripping newline
		        if(checkLength(commandToAdd)){
		        	numberOfCommands++; // increment command tracking variable
				addCommandOrPrint(commandToAdd, 1); // add the command to the list in the future
				printf("Okay, added with ID %d.\n", id);
				puts("");
				id++; // local id tracking variable incremented
				break;
		        }else{
		        	puts("Only 32 arguments allowed and 128 charcaters or less.");
		        }
		}
            } else if (!strncmp(input, "c", 1)) {//option c
                puts("");
                puts("-- Change Directory --");
                printf("New Directory?: ");
                fgets(dirChange, sizeof(dirChange), stdin); // get user input, strip newline
                dirChange[strlen(dirChange) - 1] = '\0';
                chdir(dirChange); 
            } else if (!strncmp(input, "e", 1) || check_EOF == NULL) {//option e
		waitPid = wait3(&status, WNOHANG, &usage); // checking for waiting process, if so get its pid 
		if(waitPid == 0){  // if not...
		    puts("");
		    puts("Unable to log out, jobs not completed. Waiting...");
		    for (i = 0; i < bgpIndex; i++){ // check each bg process and wait for it, if need be	
			processExit(wait4(bgProcesses[i].pid, &status, 0, &usage)); // exit the process gracefully if need be
		    }
                }
		else {
		    processExit(waitPid); // exit any processes we catch gracefully
		}
	        puts("");
                puts("Logging you out, Commander.");
                exit(0);
            } else if (!strncmp(input, "p", 1)) {//option p
                char pwd[2048];
                puts("");
                puts("-- Current Directory --");
                printf("Directory: ");
                getcwd(pwd, sizeof(pwd));
                printf("%s\n", pwd);
            } else if (!strncmp(input, "r", 1)) {//option p
                puts("");
                puts("-- Background Processes --");
                for (i = 0; i < bgpIndex; i++) { // iterate through struct and find the running ones
		    if (bgProcesses[i].running == 1) {
			printf("\n[%d] %d %s\n", bgProcesses[i].bgid, bgProcesses[i].pid, bgProcesses[i].command); // print out the info if its running
		    }
		}    
            } else {
                for (i = 0; i < 32; i++) {//check if added command exists before dismissing it as not.The user added command is also executed from here
                    if ((input[0] - '0') == commandIDs[i]) {
                        addedCommandExists = 1;
                    }
                }

                if (addedCommandExists) {
                    for (i = 0; i < 32; i++) {
                        if ((input[0] - '0') == commandIDs[i]) {//search for added command to execute
                            userCommandExec(i);//execute user added command using ID fro command
                        }
                    }
                    addedCommandExists = 0; // flip flag if command does not exist
                } else {
                    fprintf(stderr, "\nYou have entered an incorrect option.\n\n"); // You are the weakest link.
                }

            }
        }//end wait
    }
    return 0;
}

/*
Additional Files:		Makefile, README.txt, mc0.c, mc1.c

Test Procedures:		testfile is test_part3.txt

References:		     
 */

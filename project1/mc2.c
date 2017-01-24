/*
Author:				Ed Burnham, Mike Caldwell
Date:				1/18/2017
Version:			1
Project ID:			Project 1
CS Class:			CS 3013
Programming Language:		C 
OS/Hardware dependencies:	Uses Linux system calls

Problem Description:		Midday Commander - text based system interaction

Overall Design:			main design decisions about the program
	System structure	
	Data representation	
	Algorithms 		

Program Assumptions 
      and Restrictions:		what needs to be true for the program to work

Interfaces:			shell
	programs
	User
	File/D-B
	Program/Module

Implementation Details:
	Data			implementation details of data representation
	Variables		key variables and their scopes
	Algorithm		implementation details of algorithm(s) used

How to build the program:	the command line or the makefile

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

#define ARG_LIMIT 32
#define ARG_LENGTH 128

int numberOfCommands = -1;
int lastTimeMaj = 0;
int lastTimeMin = 0;
int status;

double wall_milli;

struct timeval start, stop;
struct rusage usage;
struct bgProcess {
    int bgid;
    int pid;
    char *command;
};

struct bgProcess bgProcesses[ARG_LIMIT];

char commandsToPrint[ARG_LIMIT][ARG_LENGTH];//user added commands
int commandIDs[ARG_LIMIT];//command IDs corresponding to the user added commands
int filedes[2];
int errdes[2];
int bgpIndex;
int maxReached = 0;

void addCommandOrPrint(char *command, int addOrPrint) {//0 passed to addOrPrint just prints the user added commands
  int i, j, k;                                          //1 passed to addOrPrint only adds the command to the list

    if(maxReached) {
      k = 7;
    }
    else {

      k = (numberOfCommands + 1);
    }
      
    if (addOrPrint) {//add command
      strncpy(commandsToPrint[numberOfCommands], command, 128);//copy the user added command to the list
      commandIDs[numberOfCommands] = numberOfCommands + 3;

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
    addCommandOrPrint(" ", 0);
    puts("   a. add command : Adds a new command to the menu.");
    puts("   c. change directory : Changes process working directory ");
    puts("   e. exit : Leave Mid-Day Commander");
    puts("   p. pwd : Prints working directory ");
    puts("   r. running processes : Print list of running processes");
    printf("Option?: ");
}

void printStats() {//print statistics function
    wall_milli = 0;
    puts("");
    puts("-- Statistics --");
    wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec) / 1000000.0);
    printf("Elapsed time: %f milliseconds\n", 1000.0 * wall_milli);
    getrusage(RUSAGE_CHILDREN, &usage);
    printf("Page Faults: %ld\n", (usage.ru_majflt - lastTimeMaj));
    printf("Page Faults (reclaimed): %ld\n", (usage.ru_minflt - lastTimeMin));
    puts("");
    lastTimeMaj = usage.ru_majflt;//time to subtract from cumulative time held in struct usage
    lastTimeMin = usage.ru_minflt;
}

void processExit(int waitPid) {
    char stdbuffer[4096];
    char errbuffer[4096];
    int bgIndex, i;
    
    for (i = 0; i < 32; i++) {
         if (bgProcesses[i].pid == waitPid) {
	    bgIndex = i;
         }
    }
    
    printf("-- Job Complete [%d] --\n", bgProcesses[bgIndex].bgid);
    printf("Process ID: %d\n", bgProcesses[bgIndex].pid);
    read(filedes[0], stdbuffer, sizeof(stdbuffer));
    read(errdes[0], errbuffer, sizeof(errbuffer));
    printf("%s\n", stdbuffer);
    printf("%s\n", errbuffer);
    gettimeofday(&stop, NULL);
    printStats();
}

void userCommandExec(int commandIndex) {//executes user added command
    int i, waitPid;
    int tokens = 0;
    int tempIndex = 0;

    char tempCommand[ARG_LIMIT + 1][ARG_LENGTH];//temps for parsing and copying
    char tempCommand2[ARG_LIMIT + 1][ARG_LENGTH];
    char *args[ARG_LIMIT + 1] = {NULL};//execvp needs a array of char*

	memset(tempCommand, 0, sizeof(tempCommand[0][0]) * (ARG_LIMIT + 1) * ARG_LENGTH);
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
        bgpIndex++;
	printf("%d", bgpIndex);
        args[tokens - 1] = NULL; // stripping off '\n' char
        pipe(errdes); // opening stderr pipe
        pipe(filedes); // opening stdout pipe
        gettimeofday(&start, NULL); //getting time
        bgProcesses[commandIndex].pid = fork(); // forking, storing PID in ids
        bgProcesses[commandIndex].bgid = bgpIndex;
        bgProcesses[commandIndex].command = commandsToPrint[commandIndex];

        if (bgProcesses[commandIndex].pid == 0) { // if CHILD
            while ((dup2(filedes[1], STDOUT_FILENO) == -1)) {} // dup STDOUT filedescriptor to filedes array
            close(filedes[1]); // closing in child so it available to parent
            close(filedes[0]);
            while ((dup2(errdes[1], STDERR_FILENO) == -1)) {} // same for stderr
            close(errdes[1]);
            close(errdes[0]);
            execvp(args[0], args); // executing command in background
        } else {
            close(filedes[1]);
            close(errdes[1]);
            printf("\n-- Command: %s --\n", commandsToPrint[commandIndex]);
            printf("[%d] %d\n\n", bgpIndex, bgProcesses[commandIndex].pid);
            while (1) {
                waitPid = wait3(&status, WNOHANG, &usage);
                if (waitPid > 0) { // may have to change it to ids[blah] == ...
		  processExit(waitPid);
                  break;
		} else {
                    break;
                }
            }
        }
    } else {
        gettimeofday(&start, NULL); //getting time
        if (fork() == 0) {
	    execvp(args[0], args);
        } else {
            printf("\n-- Command: %s --\n", commandsToPrint[commandIndex]);            
            while (1) {
                if (wait3(&status, 0, &usage) > 0) {
                  gettimeofday(&stop, NULL);
                  printStats();
                  break;
                }
            }
        }
    }

}

int main(int argc, char *argv[]) {
    char input[128];
    char inpath[128];
    char inarg[128];
    char dirChange[128];
    char commandToAdd[32];
    char *check_EOF;
    int id = 3;
    int waitPid = 0;
    int addedCommandExists = 0;
    int i;

    puts("===== Mid-Day Commander, v2 =====");

    while (1) {

        waitPid = wait3(&status, WNOHANG, &usage);
        if (waitPid > 0) {
            processExit(waitPid);
        } else {
            printOptions();
            check_EOF = fgets(input, sizeof(input), stdin);
	    input[strlen(input) - 1] = '\0';
	    if (strlen(input) > 1) {//if the argument is greater than 2 long, then it doesn't exist
	      fprintf(stderr, "\nYou have entered an incorrect option.\n\n");
	    }
            else if (!strncmp(input, "0", 1)) {//option 0
                puts("");
                puts("-- Who Am I? --");
                gettimeofday(&start, NULL);
                system("whoami");
                gettimeofday(&stop, NULL);
                printStats();

            } else if (!strncmp(input, "1", 1)) {//option 1
                puts("");
                puts("-- Last Logins --");
                gettimeofday(&start, NULL);
                system("last");
                gettimeofday(&stop, NULL);
                printStats();
            } else if (!strncmp(input, "2", 1)) {//option 2
                puts("");
                puts("-- Directory Listing --");
                printf("Arguments?: ");
                fgets(inarg, sizeof(inarg), stdin);
                printf("Path?: ");
                fgets(inpath, sizeof(inpath), stdin);
				
                if ((inarg[0] == '\n') && (inpath[0] != '\n')) {
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
                } else {
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
	      if (id == 10) {
		puts("You've reached the maximum number of user commands. New commands overwrite old ones. ");
		id = 3;
		numberOfCommands = -1;
		maxReached = 1;
	      }
	      puts("");
	      puts("-- Add a command --");
                printf("Command to add?: ");
                fgets(commandToAdd, sizeof(commandToAdd), stdin);
                commandToAdd[strlen(commandToAdd) - 1] = '\0';
                numberOfCommands++;
                addCommandOrPrint(commandToAdd, 1);
                printf("Okay, added with ID %d.\n", id);
                puts("");
                id++;
            } else if (!strncmp(input, "c", 1)) {//option c
                puts("");
                puts("-- Change Directory --");
                printf("New Directory?: ");
                fgets(dirChange, sizeof(dirChange), stdin);
                dirChange[strlen(dirChange) - 1] = '\0';
                chdir(dirChange);
            } else if (!strncmp(input, "e", 1) || check_EOF == NULL) {//option e
	      if(wait3(&status, WNOHANG, &usage) == 0){
		puts("");
		puts("Unable to log out, jobs not completed. Waiting...");
                for(i = 0; i < numberOfCommands + 1; i++){
                    processExit(wait4(bgProcesses[i].pid, &status, WNOHANG, &usage));
                  }
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
                if (fork() == 0) {
                    puts("");
                    execl("/bin/ps", "ps", "-e", (char *) NULL);
                } else {
                    wait(NULL);
                    puts("");
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
                    addedCommandExists = 0;
                } else {
                    fprintf(stderr, "\nYou have entered an incorrect option.\n\n");
                }

            }
        }//end wait
    }
    return 0;
}

/*
Additional Files:		names of other files used

Results:			listing of sample run(s) of the program

Test Procedures:		how the program was tested
Test Data:			test cases

Performance Evaluation:		how well the program performs
	Time/Space
	User Interface

References:			books, papers, people, web, ...
 */



/*
  Author:			Ed Burnham, Mike Caldwell
  Date:				1/16/2017 - 1/20/2017
  Version:			1
  Project ID:			Project 1
  CS Class:			CS 3013
  Programming Language:		C 
  OS/Hardware dependencies:	Uses Linux system calls

  Problem Description:		Midday Commander - text based system interaction

  Overall Design:			
  System structure	The program has a flat structure with a main function and a few small functions called from main
  Data representation	most data is represented in primitive variable types. 
  Algorithms 		The program makes no use of any algorithms worth mentioning

  Program Assumptions 
  and Restrictions:		There are limits on user input, details available in README

  Interfaces:			system calls, stdin, stderr, stdout

  Implementation Details:
		        
  Functions	        Use of fork(), exec() and wait() functions for process control

  How to build the program:	make all
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

#define ARG_LIMIT 32 // limit on number of args
#define ARG_LENGTH 128 // limit on length of an arg

int numberOfCommands = -1;
int lastTimeMaj = 0;  // ints for storing cumulative time of children
int lastTimeMin = 0;
int maxReached = 0;
int status = 0;

double wall_milli = 0; // wall clock timer

struct timeval start, stop;
struct rusage usage;

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

void userCommandExec(int commandIndex){
    int i;
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
            tempIndex = i + 1;//keeping track of i so the next array fills from element zero
            tokens++;//move to the next string in array if strings
        } else {
            tempCommand2[tokens][i - tempIndex] = tempCommand[0][i];//copy character by character
        }
    }
    args[tokens - 1] = NULL; // stripping off '\n' char      	   	
    gettimeofday(&start, NULL); //getting time
    if (fork() == 0) { // if CHILD
	execvp(args[0], args);// executing command in background
    }
    else { // in PARENT
	wait(&status); // wait for child
	gettimeofday(&stop, NULL); // end stopwatch
	printStats(); 
    }		
}

int checkLength(char* command){ // checking for erroneous input 
    int count = 0;
    int i;
    for (i = 0; i < strlen(command) + 1; i++) { // counting spaces
	if (command[i] == ' ') {
	    count++;
	}
    }
    
    if(strlen(command) > 126){ // checking length, minus newline char
	return 0;
    }
    else if(count > 32){ // if there are more than 32 arguements
     	return 0;
    }
    else{
    	return 1; // otherwise, good to go
    }
}

int main(int argc, char *argv[]){
    char input[129]; // buffers for user input
    char inpath[129];
    char inarg[129];
    char commandToAdd[129]; // string of the user added command
    char dirChange[129]; // input buffer for chdir command
    char *check_EOF; // checking EOF when files are passed as input
    int id = 3;
    int addedCommandExists = 0; // flag for seeing if a command has been added
    int i;

    puts("===== Mid-Day Commander, v1 =====");
	 
    while(1) {
	printOptions();
	
	check_EOF = fgets(input, sizeof(input), stdin); // get input from user and save EOF for checking later
	input[strlen(input) - 1] = '\0';
	if (strlen(input) > 1) {//if the argument is greater than 1 long, then it doesn't exist
	    fprintf(stderr, "\nYou have entered an incorrect option.\n\n");
	}
	else if(!strncmp(input, "0", 1)){ // option 0
	    puts("");
	    puts("-- Who Am I? --");
	    gettimeofday(&start, NULL);
	    system("whoami"); // system call to exec whoami
	    gettimeofday(&stop, NULL);
	    printStats();
	}
	else if(!strncmp(input, "1", 1)){ // option 1
      	    puts("");
	    puts("-- Last Logins --");
	    gettimeofday(&start, NULL);
	    system("last");
	    gettimeofday(&stop, NULL);
	    printStats();
	}
	else if (!strncmp(input, "2", 1)) {//option 2
	    puts("");
	    puts("-- Directory Listing --"); // getting path and args of ls from user
	    printf("Arguments?: ");
	    fgets(inarg, sizeof(inarg), stdin);
	    printf("Path?: ");
	    fgets(inpath, sizeof(inpath), stdin);
	    if (checkLength(inpath) && checkLength(inarg)) {		
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
	    }
	    else {
		puts("");
		puts("ERROR: Maximum 32 arguments and 128 characters allowed.");
		puts("");
	    }
	}
	else if (!strncmp(input, "a", 1)) {//option a
	    if (id == 10) { // checking if the maximum user added command limit has been reached
		puts("You've reached the maximum number of user commands. New commands overwrite old ones. ");
		id = 3; // reseting local tracking variables
		numberOfCommands = -1;
		maxReached = 1; // flag for maximum reached
		puts("");
	    }
		
	    puts("");
	    puts("-- Add a command --");
	    printf("Command to add?: ");
	    fgets(commandToAdd, sizeof(commandToAdd), stdin); // get command to add from user
	    commandToAdd[strlen(commandToAdd) - 1] = '\0'; // stripping newline
	    if(checkLength(commandToAdd)){ // making sure user input comforms to expectations
		numberOfCommands++; // increment command tracking variable
		addCommandOrPrint(commandToAdd, 1); // add the command to the list in the future
		printf("Okay, added with ID %d.\n", id);
		puts("");
		id++; // local id tracking variable incremented
	    }
	    else{
		puts("");
		puts("ERROR: Maximum 32 arguments and 128 characters allowed.");
		puts("");
	    }
	} else if (!strncmp(input, "c", 1)) {//option c
	    puts("");
	    puts("-- Change Directory --");
	    printf("New Directory?: ");
	    fgets(dirChange, sizeof(dirChange), stdin); // get user input, strip newline
	    dirChange[strlen(dirChange) - 1] = '\0';
	    chdir(dirChange);
	    puts("");
	} else if (!strncmp(input, "e", 1) || check_EOF == NULL) {//option e
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
	    puts("");
	}
	else{
	    for(i = 0; i < 32; i++){
		if((input[0] - '0') == commandIDs[i]){ // checking to see if a command id exits already
		    addedCommandExists = 1;
		}
	    }	
    
	    if(addedCommandExists){ 
		for(i = 0; i < 32; i++){
		    if((input[0] - '0') == commandIDs[i]){//search for added command to execute
			userCommandExec(i);
		    }
		}
		addedCommandExists = 0;
	    }
	    else{
		fprintf(stderr, "You have entered an incorrect option.\n");
	    }	
	}
    }
    return 0;
}

/*
  Additional Files:		Makefile, README.txt, mc0.c, mc1.c, mc2.c, test_mc0.txt, test_mc1.txt, test_mc2.txt

  Test Procedures:		testfile is test_mc1.txt

  References:	cplusplus.com
  man7.org
			     
*/

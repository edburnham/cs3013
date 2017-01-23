/*
Author:				Ed Burnham, Mike Caldwell
Date:				1/18/2017
Version:			1
Project ID:			Project 1 - Checkpiont
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

Program Source:			listing of the source code

Additional Files:		names of other files used

Results:			listing of sample run(s) of the program

Test Procedures:		how the program was tested
Test Data:			test cases

Performance Evaluation:		how well the program performs
	Time/Space
	User Interface

References:			books, papers, people, web, ...
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

int numberOfCommands = -1;
int lastTimeMaj = 0; 
int lastTimeMin = 0;

double wall_milli;

struct timeval start, stop;
struct rusage usage;

char commandsToPrint[32][128];
int commandIDs[32];

void addCommandOrPrint(char* command, int addOrPrint){
	int i, j;
	
	if(addOrPrint){//add command
		strncpy(commandsToPrint[numberOfCommands], command, 128);
		commandIDs[numberOfCommands] = numberOfCommands + 3;
	}
	else{//don't add command just print them 
		for(i = 0; i < (numberOfCommands + 1); i++){
			printf("   %d. ", commandIDs[i]);
			for(j = 0; commandsToPrint[i][j] != '\0'; j++){
				printf("%c", commandsToPrint[i][j]);
			}
			puts("      : User added command");
		}
	}
}

void printOptions(){
	puts("G'day, Commander! What command would would you like to run");
	puts("   0. whoami   : Prints out the result of the whoami command");
	puts("   1. last     : Prints out the results of the last command");
	puts("   2. ls       : Prints out the result of a listing on a user specified path");
	addCommandOrPrint(" ", 0);
	puts("   a. add command : Adds a new command to the menu.");
  	puts("   c. change directory : Changes process working directory ");
	puts("   e. exit : Leave Mid-Day Commander");
	puts("   p. pwd : Prints working directory ");
	printf("Option?: ");
}

void printStats(){
	wall_milli = 0;
	puts("");
	puts("-- Statistics --");
	wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec)/1000000.0);
	printf("Elapsed time: %f milliseconds\n", wall_milli);
	getrusage(RUSAGE_CHILDREN, &usage);	
	printf("Page Faults: %ld\n", (usage.ru_majflt - lastTimeMaj));
	printf("Page Faults (reclaimed): %ld\n", (usage.ru_minflt - lastTimeMin));
	puts("");
	lastTimeMaj = usage.ru_majflt;
	lastTimeMin = usage.ru_minflt;
}

void userCommandExec(int commandIndex){
	int i;
	int put = 0;
	int status;
	int temp = 0;
	
	char parsedCommand[32][128];
	char tempCommand[32][128];
	char tempCommand2[32][128];
	char* args[] = {parsedCommand[1], NULL};
	
	strcpy(tempCommand[0], commandsToPrint[commandIndex]);
	tempCommand[0][127] = '\0';	
	
	for(i = 0; i < strlen(tempCommand[0]) + 1; i++){
		if((tempCommand[0][i] == ' ') || (tempCommand[0][i] == '\0')){
			strcpy(parsedCommand[put], tempCommand2[put]);
			temp += i;
			temp++;
			put++;
		}
		else{
			tempCommand2[put][i - temp] = tempCommand[0][i];
		}
	}
	      	   	
	gettimeofday(&start, NULL); 
	if (fork() == 0) {
	   execvp(parsedCommand[0], args);
	}
	else {
	   wait(&status);
	   gettimeofday(&stop, NULL);
	   printStats();
	}		
}

int main(int argc, char *argv[]){
	char input[128];
	char inpath[128];
	char inarg[128];
	char commandToAdd[32];
	
	int status;
	int id = 3;
	int addedCommandExists = 0;
	int i;
	//int flag = 0;

	puts("===== Mid-Day Commander, v1 =====");
	 
	while(1) {
	  printOptions();
	
	  fgets(input, sizeof(input), stdin);
	  input[strlen(input) - 1] = '\0';
	  if(!strncmp(input, "0", 1)){
	    puts("");
	    puts("-- Who Am I? --");
	    gettimeofday(&start, NULL);
	    system("whoami");
	    gettimeofday(&stop, NULL);
	    printStats();

	  }
	  else if(!strncmp(input, "1", 1)){
      	    puts("");
	    puts("-- Last Logins --");
	    gettimeofday(&start, NULL);
	    system("last");
	    gettimeofday(&stop, NULL);
	    printStats();
	  }
	  else if(!strncmp(input, "2", 1)){//Option 2
	    puts("");
	    puts("-- Directory Listing --");

	    printf("Arguments?: ");
	    fgets(inarg, sizeof(inarg), stdin);

	    /* if(strlen(inarg) == 1){
	    	flag = 1;
		}*/
	    
	    printf("Path?: ");
	    fgets(inpath, sizeof(inpath), stdin);

	    gettimeofday(&start, NULL); 
	    /*	    if(flag){
	      if (fork() == 0) {
		puts("");
      	        execl("/bin/ls", "ls", inpath, (char *) NULL);
      	      }
	      else {
		wait(&status);
		gettimeofday(&stop, NULL);
		printStats();
	      }
	    }
	    else {*/
	      if (fork() == 0) {
		puts("");
      	      	execl("/bin/ls", "ls", inarg, inpath, (char *) NULL);
	      }
	      else {
		wait(&status);
		gettimeofday(&stop, NULL);
		printStats();
	      }
	    
	  }
	  else if(!strncmp(input, "a", 1)){
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
	  }
	  else{
	  	for(i = 0; i < 32; i++){
			if((input[0] - '0') == commandIDs[i]){
				addedCommandExists = 1;
			}
		}
	  	
  	    if(strlen(input) > 2){
   	        fprintf(stderr, "You have entered an incorrect option.\n");
    	}
    	else if(addedCommandExists){ 
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


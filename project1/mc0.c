/*
Author:				Ed Burnham, Mike Caldwell
Date:				1/18/2017
Version:			0
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
#include <math.h>

int main(int argc, char *argv[]){
	char input[3];
	char inpath[2048];
	char inarg[2048];
	int status;
	double wall_milli;
	
	struct timeval start, stop;
	struct rusage usage;

	puts("===== Mid-Day Commander, v0 =====");
	 
	while(1) {
	  wall_milli = 0;
	  
	  puts("G'day, Commander! What command would would you like to run");
	  puts("   0. whoami   : Prints out the result of the whoami command");
	  puts("   1. last     : Prints out the results of the last command");
	  puts("   2. ls       : Prints out the result of a listing on a user specified path");
	  printf("Option?: ");

	  fgets(input, sizeof(input), stdin);
	  input[strlen(input) - 1] = '\0';

	  if(!strncmp(input, "0", 1)){
	    puts("");
	    puts("-- Who Am I? --");
	    gettimeofday(&start, NULL);
	    system("whoami");
	    gettimeofday(&stop, NULL);
	    puts("");
	    puts("-- Statistics --");
	    wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec)/1000000.0);
	    printf("Elapsed time: %f milliseconds\n", wall_milli);
	    getrusage(RUSAGE_SELF, &usage);
	    printf("Page Faults: %ld\n", usage.ru_majflt);
	    printf("Page Faults (reclaimed): %ld\n", usage.ru_minflt);
	    puts("");
	  }
	  else if(!strncmp(input, "1", 1)){
      	    puts("");
	    puts("-- Last Logins --");
	    gettimeofday(&start, NULL);
	    system("last");
	    gettimeofday(&stop, NULL);
	    puts("");
	    puts("-- Statistics --");
	    wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec)/1000000.0);
	    printf("Elapsed time: %f milliseconds\n", wall_milli);
	    getrusage(RUSAGE_SELF, &usage);
	    printf("Page Faults: %ld\n", usage.ru_majflt);
	    printf("Page Faults (reclaimed): %ld\n", usage.ru_minflt);
	    puts("");
	  }
	  else if(!strncmp(input, "2", 1)){
	    puts("");
	    puts("-- Directory Listing --");

	    printf("Arguements?: ");
	    fgets(inarg, sizeof(inarg), stdin);
	    inarg[strlen(inarg) - 1] = '\0';
	    	    
	    printf("Path?: ");
	    fgets(inpath, sizeof(inpath), stdin);
	    inpath[strlen(inpath) - 1] = '\0';

	    gettimeofday(&start, NULL); 
	    if (fork() == 0) {
	      printf("\n");
	      execl("/bin/ls", "ls", inarg, inpath, (char *) NULL);
	    }
	    else {
	      wait(&status);
	      gettimeofday(&stop, NULL);
	      puts("");
	      puts("-- Statistics --");
	      wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec)/1000000.0);
	      printf("Elapsed time: %f milliseconds\n", wall_milli);
	      getrusage(RUSAGE_SELF, &usage);
	      printf("Page Faults: %ld\n", usage.ru_majflt);
	      printf("Page Faults (reclaimed): %ld\n", usage.ru_minflt); 
	      puts("");
	    }
	  }
	  else{
	    puts("");
	    fprintf(stderr, "You have entered an incorrect option.\n");
	    puts("");
	  }
	
      	}
	return 0;
}

/*
Author:				Ed Burnham, Mike Caldwell
Date:				1/16/2017 - 1/24/2017
Version:			0
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
#include <math.h>

int main(int argc, char *argv[]){
    char input[3]; // buffers for user input
	char inpath[2048];
	char inarg[2048];
	char* checkEOF;//for end of file test file
	int status;
	double wall_milli; // wall clock storage
	
	struct timeval start, stop;
	struct rusage usage;

	puts("===== Mid-Day Commander, v0 =====");
	 
	while(1) {
	  wall_milli = 0;//for statistics
	  
	  puts("G'day, Commander! What command would would you like to run");
	  puts("   0. whoami   : Prints out the result of the whoami command");
	  puts("   1. last     : Prints out the results of the last command");
	  puts("   2. ls       : Prints out the result of a listing on a user specified path");
	  printf("Option?: ");

	  checkEOF = fgets(input, sizeof(input), stdin);//get user input
	  input[strlen(input) - 1] = '\0';
	  if(strlen(input) > 2){ // sanitizing input
	  	puts("You have entered an incorrect option.");
	  }else{

		  if(!strncmp(input, "0", 1)){//option 0
			puts("");
			puts("-- Who Am I? --");
			gettimeofday(&start, NULL); // start stopwatch
			system("whoami");//exec and fork wrapper function system()
			gettimeofday(&stop, NULL); // stop stopwatch
			puts("");
			puts("-- Statistics --");
			wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec)/1000000.0); // doing time calculation
			printf("Elapsed time: %f milliseconds\n", 1000.0 * wall_milli); // dissplaying time
			getrusage(RUSAGE_SELF, &usage); // getting page fault information
			printf("Page Faults: %ld\n", usage.ru_majflt);
			printf("Page Faults (reclaimed): %ld\n", usage.ru_minflt);
			puts("");
		  }
		  else if(!strncmp(input, "1", 1)){//option 1
		  	    puts("");
			puts("-- Last Logins --");
			gettimeofday(&start, NULL);
			system("last");
			gettimeofday(&stop, NULL);
			puts("");
			puts("-- Statistics --");
			wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec)/1000000.0);
			printf("Elapsed time: %f milliseconds\n", 1000.0 * wall_milli);
			getrusage(RUSAGE_SELF, &usage);
			printf("Page Faults: %ld\n", usage.ru_majflt);
			printf("Page Faults (reclaimed): %ld\n", usage.ru_minflt);
			puts("");
		  }
		  else if(!strncmp(input, "2", 1)){//option 2
			puts("");
			puts("-- Directory Listing --");

			printf("Arguments?: ");//get arguments for ls
			fgets(inarg, sizeof(inarg), stdin);
			inarg[strlen(inarg) - 1] = '\0';
				    
			printf("Path?: ");//gt path to run ls on
			fgets(inpath, sizeof(inpath), stdin);
			inpath[strlen(inpath) - 1] = '\0';

			gettimeofday(&start, NULL); 
			if (fork() == 0) { //  this is the CHILD
			  printf("\n");
			  execl("/bin/ls", "ls", inarg, inpath, (char *) NULL);
			}
			else { // parent
			    wait(&status); // waiting for child to complete
			  gettimeofday(&stop, NULL);
			  puts("");
			  puts("-- Statistics --");
			  wall_milli = (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec)/1000000.0);//calulate time
			  printf("Elapsed time: %f milliseconds\n", 1000.0 * wall_milli);
			  getrusage(RUSAGE_SELF, &usage);//get usage info - page faults
			  printf("Page Faults: %ld\n", usage.ru_majflt);
			  printf("Page Faults (reclaimed): %ld\n", usage.ru_minflt); 
			  puts("");
			}
		  }
		  else if(checkEOF == NULL){//check for end of file, for testing input with file
		  		puts("Logging you out, Commander.");
		  		exit(0);
		  }
		  else{
			puts("");
			fprintf(stderr, "You have entered an incorrect option.\n");
			puts("");
		  }
	  }	
      	}
	return 0;
}

/*
Additional Files:		Makefile, README.txt, mc0.c, mc1.c, mc2.c, test_mc0.txt, test_mc1.txt, test_mc2.txt

Test Procedures:		testfile is test_mc0.txt

References:	cplusplus.com
		man7.org	     
 */


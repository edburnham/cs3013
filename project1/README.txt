Edward Burnham
Michael Caldwell
CS3013 C Term 2017
Project 1 - Midday Commander

Compile using: make
To run use: ./mc2 (latest version)

This program is a text-based shell that lets the user choose from multiple executable commands and also add their own commands to the list. The user is allowed to add up to and including 7 commands. When the 8th command is added, it overwrites that first command that was added by the user. After each command is run, statistics are printed to the screen such as time to complete and number of page faults. The project was set-up in 3 phases, each of which implemented different features into the shell.

Phase 1:

In phase 1, the user has 3 commands to run: whoami, last, and ls. Since these are Linux commands they can be executed with the system() function. This type of function can be called from C programs to execute a Linux command from inside a program. The ls function required that the user provide arguments and a path. In order to accomodate these, the exec() function was used. The exec funtion allows the arguments to be passed to it. The system() funcion uses the exec function "under the hood." The ls command is run by a child process that was created with fork(). The Midday Commander ls command implementation can either take no arguments, arguments and no path, path and no arguments, and no path and no arguments. 

Phase 2:

In this phase, the user can now add commands, change their directory, exit the program, and print their working directory. The user added commands get added to a global string array of commands. It is then added to the list and displayed on the screen. When the user selects their new command to run, it is searched for by id. The id number is passed to the execute command function and the command that is in the array with the corresponding id is executed. The command is executed in the child process that is created with fork and executed with the command execvp(). Execvp can take a list of commands to execute. In order to properly give execvp an array of char*s, the arguments are parsed and put into an array of char*s. The arguments are parsed by looping through character by character. When a space is found the string is put into the arguments array. The parsing continues until the '\0' character has been reached.

To handle the adding and printing of the commands a function was created that takes in flag that is either 0 or 1. When the flag is 1 the command is added to the list of commands. When the flag is 0, the list of added commands are printed. The function is both utilized when printing the list of options (all the user commands will be printed)

The change directory function was iplemented with the chdir() C function.

The exit command "e" makes sure there are no running process that are running before logging out and printing a message. The program sits in loop that checks each process id with wait to see if it has finished.

The print woring directiry command was implemeted by getting the current working directry by using getcwd(), a C function, and writing the result to stdout. 

Phase 3:

In this phase, the shell is modified to handle background tasks, indicated by a '&'. When the user adds a command that must run in the background, the process must fork to create a child process for that command. As the commands are being executed user input can still be read from stdin. Furthermore, when a background task finishes, its statistcs are printed to the screen as well as a notification of which job has finished. 

A command 'r' was added to the list that prints out the background processes that are running. Theses processes were spawned with Midday Commander. In order to keep track of these processes and control when their statistics would print, the wait3 and wait4 functions were utilized with and with out the WNOHANG argument present. Since the statistics are to be printed out when the command has finished, the command needed to be waited for. 

Since the user input was gathered by using fgets, the output does not update until a command is run. In future projects, the select() might be a better choice because the select function can check if the user has input anything on stdin.

Data Structures:
The background processes' information is kept in an array of structs. Each element in the array has a job id, process id, the command, and a flag that is set when the command is executing. An else-if chain is used to implement the option slection. Functions were created to reduce the amount of code that would have otherwise been repeated such as printing the options menu. 

Testing:
The testing was done by manually entering in the options, executing commands, and entering reasonable edge cases such as when the user decides not to enter in a path argument. A text file was generated for testing for each part. Also, the program terminates when the end-of-file of reached, as a part of the project directions.


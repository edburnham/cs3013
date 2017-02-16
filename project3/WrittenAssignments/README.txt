Edward Burnham
Michael Caldwell

README for Project 3B

We have provided a constant which can be edited in macsem.c or macmutex.c called EXP_DURATION which will define the amount of time in seconds you want the simulation to run. Every dwell_duration a counter is incremented and compared with EXP_DURATION. When the two values are equal, the program terminates. You can also define NUM_NODES and NUM_NOISE_MAKERS in the same location. The number of noise makers is a subset of the total number of nodes (NUM_NODES). Macros also define the thread settings such as dwell_probability and talk_probability. When you have set your desired settings you can run "make all" to compile both semaphore and mutex programs. Run either macsem for semaphores or macmutex for locks/condition variables. When the program runs it will display a grid along with ta corresponding table indicating with node on the grid is which nodeID. Choose a cluster of interest and use one of the node numbers to look up the corresponding node ID in the table. You can then examine the log file which is named <NODE_ID>.txt in the folder you ran the program in. This logfile contains a detailed description of the environment the node was simulated in. You can use this information to trace a message back to its source. Noisemakers create a log file that have the name noisemaker<nodeID>.txt that can be found in the directory of which the program was ran. The noise maker log file indicates the time of day interval and channel the noisemaker was on during its running time. 

The program takes no input files for tetsing. All tests are performed by changing the variables or MACROS at the top of each file.

There is also additional information found in the problem description regaurding assumptions about the experient.

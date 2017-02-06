//Edward Burnham
//Michael Caldwell

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

#define SEED_VALUE 10

double num_noisemakers = 0;
double dwell_noisemakers = 0;
double dwell_probability_noisemakers = 0;

/*random messages to send*/
char* mess[10] = {"hello", "goodbye", "cs3013", "orange", "purple", "michael!", "edward!", "johnson!", "watson!", "robinson!"};

typedef struct message{
	int sourceID;
	char* transMess;
	int channel;
	int x_coor;
	int y_coor;
}message;

/*thread data*/
typedef struct nodeData{
	unsigned int ID_self;
	int is_noisemaker;
	int x_coor;
	int y_coor;
	double dwell_duration;
	double dwell_probability;
	double transmission_time;
	double talk_window_time;
	double talk_probability;
	double dwell_noisemakers;
	double dwell_probability_noisemakers;
}nodeData;

/*thread process*/
void* nodeProcess(void* stuff){
	nodeData* data = (nodeData*)stuff;
	/*initializes coordinates for node*/
	
	return 0;
}

int main(int argc, char** argv){
	srand((unsigned int)SEED_VALUE);
	int numNodes = atoi(argv[1]);
	int num_noisemakers = atoi(argv[2]);
	int i, checkError;
	
	pthread_t nodes[numNodes];
	nodeData nodeInfo[numNodes];//data struct for each thread
	
	if(argc == 3){
		for(i = 0; i < numNodes; i++){
			nodeInfo[i].ID_self = rand() % UINT_MAX;//set node Id
			nodeInfo[i].x_coor = rand() % 99;
			nodeInfo[i].y_coor = rand() % 99;
			nodeInfo[i].dwell_duration = 1;
			nodeInfo[i].dwell_probability = 1;
			nodeInfo[i].transmission_time = 1;
			nodeInfo[i].talk_window_time = 1;
			nodeInfo[i].talk_probability = 1;
			if(i < num_noisemakers){//make desired amount of nodes noisemakers
				nodeInfo[i].is_noisemaker = 1;
				nodeInfo[i].dwell_noisemakers = 1;
				nodeInfo[i].dwell_probability_noisemakers = 1;
			}else{
				nodeInfo[i].is_noisemaker = 0;
				nodeInfo[i].dwell_noisemakers = 0;
				nodeInfo[i].dwell_probability_noisemakers = 0;
			}
			if((checkError = pthread_create(&nodes[i], NULL, nodeProcess, &nodeInfo[i]))){
				fprintf(stderr, "Failed to create thread with pthread_create.\n");
				return 0;
			}
		}
	}else{
		puts("Correct usage: ./mac <number of nodes> <number of noise makers>");
	}
	return 0;
}
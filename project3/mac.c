//Edward Burnham
//Michael Caldwell

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <limits.h>

#define SEED_VALUE 10
#define NUM_NODES 100
#define NUM_NOISE_MAKERS 0

/*random messages to send*/
char* mess[15] = {"hello", "goodbye", "cs3013", "orange", "purple", "michael!", "edward!", "johnson!", "watson!", "danger!", "stranger!", "things!", "DOOM!", "highlander!", "whatThe!"};

pthread_mutex_t channel_1lock;
pthread_mutex_t channel_6lock;
pthread_mutex_t channel_11lock;

typedef struct message{
	unsigned int nodeID;
	char* transMess;
	int channel;
	int x_coor;
	int y_coor;
}messageData;

/*channels*/
messageData channel_1[1000];
messageData channel_6[1000];
messageData channel_11[1000];

messageData nodeCache[NUM_NODES];

/*thread data*/
typedef struct nodeData{
	unsigned int nodeID;
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
void* nodeProcess(void* nodeInfo){
	nodeData* data = (nodeData*)nodeInfo;
	//char* message;
	unsigned int IDs_inrange[100] = {0};
	//char* messageBuff[100];
	int i;
	printf("data node ID: %d\n", data->nodeID);
	//printf("0 ID: %d\n1 ID: %d\n", nodeCache[0].nodeID, nodeCache[1].nodeID);
	for(i = 0; i < NUM_NODES; i++){// && (data->y_coor <= nodeCache[i].y_coor + 5) && (data->y_coor >= nodeCache[i].y_coor - 5)
		
			//printf("xcoor: %d ycoor: %d\n", nodeCache[i].x_coor, nodeCache[i].y_coor);
			if((data->x_coor <= nodeCache[i].x_coor + 5) && (data->x_coor >= nodeCache[i].x_coor - 5)&& (data->y_coor <= nodeCache[i].y_coor + 5) && (data->y_coor >= nodeCache[i].y_coor - 5)){
				IDs_inrange[i] = nodeCache[i].nodeID;//in range
			}
			else if((nodeCache[i].x_coor < 5 && data->x_coor < 5) && (nodeCache[i].y_coor > 95 && data->y_coor > 95) && (data->y_coor < (nodeCache[i].y_coor + 5))){
				IDs_inrange[i] = nodeCache[i].nodeID;//in range
			}
			else if((nodeCache[i].x_coor > 95 && data->x_coor > 95) && (nodeCache[i].y_coor < 5 && data->y_coor < 5) && (data->y_coor > (nodeCache[i].y_coor - 5))){
				IDs_inrange[i] = nodeCache[i].nodeID;//in range
			}
			else if((nodeCache[i].x_coor > 95 && data->x_coor > 95) && (nodeCache[i].y_coor > 95 && data->y_coor > 95) && (data->y_coor > (nodeCache[i].y_coor - 5))){
				IDs_inrange[i] = nodeCache[i].nodeID;//in range
			}
			else if((nodeCache[i].x_coor < 5 && data->x_coor < 5) && (nodeCache[i].y_coor < 5 && data->y_coor < 5) && (data->y_coor < (nodeCache[i].y_coor + 5))){
				IDs_inrange[i] = nodeCache[i].nodeID;//in range
			}

	}
	
	for(i = 0; i < NUM_NODES; i++){
		if(IDs_inrange[i] != 0){
				printf("threadID: %d -- ID in range: %d\n", data->nodeID, IDs_inrange[i]);

		}
	}
	//pthread_mutex_lock(&channel_1lock);
	//pthread_mutex_unlock(&channel_1lock)
	
	/*
	while(1){
		
		
		//message = strdup(mess[rand() % 14]); 
	}*/
	
	pthread_exit(NULL);
}

int main(int argc, char** argv){
	srand(time(NULL));
	int i, checkError;
	
	pthread_t nodes[NUM_NODES];
	nodeData nodeInfo[NUM_NODES];//data struct for each thread
	
	if(argc == 1){
		for(i = 0; i < NUM_NODES; i++){
			nodeInfo[i].nodeID = rand() % UINT_MAX;//set node Id, 4 bytes
			//nodeInfo[i].nodeID = i;//for testing
			nodeCache[i].nodeID = nodeInfo[i].nodeID;//line stay for testing and non testing
			
			/*non-testing coordinate generation*/
			nodeInfo[i].x_coor = rand() % 99;
			nodeInfo[i].y_coor = rand() % 99;
			nodeCache[i].x_coor = nodeInfo[i].x_coor;
			nodeCache[i].y_coor = nodeInfo[i].y_coor;
			
			/*testing mode coordinate generation
			nodeInfo[i].x_coor = 95 + 5*i ;
			nodeInfo[i].y_coor = 5*i; 
			nodeCache[i].x_coor = nodeInfo[i].x_coor;
			nodeCache[i].y_coor = nodeInfo[i].y_coor;*/
			
			nodeInfo[i].dwell_duration = 1.0;
			nodeInfo[i].dwell_probability = 1.0;
			nodeInfo[i].transmission_time = 1.0;
			nodeInfo[i].talk_window_time = 1.0;
			nodeInfo[i].talk_probability = 1.0;
			if(i < NUM_NOISE_MAKERS){//make desired amount of nodes noisemakers
				nodeInfo[i].is_noisemaker = 1;
				nodeInfo[i].dwell_noisemakers = 1.0;
				nodeInfo[i].dwell_probability_noisemakers = 1.0;
			}else{
				nodeInfo[i].is_noisemaker = 0;
				nodeInfo[i].dwell_noisemakers = 1.0;
				nodeInfo[i].dwell_probability_noisemakers = 1.0;	
			}
			//printf("ID: %d\n", nodeInfo[i].nodeID);
			//puts("sfgsdfhswetg!!!!!");
			if((checkError = pthread_create(&nodes[i], NULL, nodeProcess, &nodeInfo[i]))){
				fprintf(stderr, "Failed to create thread with pthread_create().\n");
				return 0;
			}
			//puts("hhhh");
		}
		
		for (i = 0; i < NUM_NODES; ++i) {
    		pthread_join(nodes[i], NULL);
  		}
	}else{
		puts("Correct usage: ./mac");
	}
	
	return 0;
}
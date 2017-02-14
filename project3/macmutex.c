//Edward Burnham
//Michael Caldwell

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <sys/types.h>
#include <semaphore.h>

#define NUM_NODES 200
#define NUM_NOISE_MAKERS 0
#define EXP_DURATION 20 //Experiment duration ~ dwell duration * EXP_DURATION
#define CHANNEL_SIZE 4000 //number of messages each channel can hold
#define MESSAGE_BUFF_SIZE 300

/*random messages to send*/
char* mess[25] = {"hello!", "goodbye!", "cs3013!", "orange!", "purple!",\
			      "michael!", "edward!", "johnson!", "watson!", "danger!",\
				  "stranger!", "things!", "DOOM!", "highlander!", "whatThe!",\
				  "mutual!", "exclusion!", "friend!", "semaphore!", "great!",\
				  "disturbed!", "demented!", "torn!", "weird!", "why!"};

/*POSIX thread mutex locks*/
pthread_mutex_t channel_1lock;
pthread_mutex_t channel_6lock;
pthread_mutex_t channel_11lock;

/*message struct*/
typedef struct message{
	unsigned int nodeID;
	char transMess[50];
	int channel;
	int x_coor;
	int y_coor;
	char time[28];
}messageData;

/*channels*/
messageData channel_1[CHANNEL_SIZE];
messageData channel_6[CHANNEL_SIZE];
messageData channel_11[CHANNEL_SIZE];
int channel_1count = 0;
int channel_6count = 0;
int channel_11count = 0;

messageData nodeCache[NUM_NODES];//for determining who is in range of who

/*thread data*/
typedef struct nodeData{
	unsigned int nodeID;
	int is_noisemaker;
	int x_coor;
	int y_coor;
	int dwell_duration;
	int dwell_probability;
	int transmission_time;
	int talk_window_time;
	int talk_probability;
	int dwell_noisemakers;
	int dwell_probability_noisemakers;
}nodeData;

/*thread process*/
void* nodeProcess(void* nodeInfo){
	nodeData* data = (nodeData*)nodeInfo;
	time_t currentTime;
	
	int messageBuffCount = 0;
	int logBuffCount = 0;
	int channel = random() % 3;
	int totalTime = 0;
	int numNodesInRange = 0;
	int i, j, k;
	int localChannelCount1 = 0;
	int localChannelCount6 = 0;
	int localChannelCount11 = 0;
	int sawMess_ch1 = 0;
	int sawMess_ch6 = 0;
	int sawMess_ch11 = 0;
	
	char uniqueMess[50];//ID concatenated with message
	char filename[50];
	
	/*keeping track of nodes in range*/
	typedef struct inRange{
		unsigned int nodeID;
		int x_coor;
		int y_coor;
	}inRange;

	messageData messageBuffer[100];
	messageData logBuffer[500];//messages to write to buffer

	inRange nodesInRange[100];

	/*Locator algorithm - checks which nodes are in rage of each other*/
	for(i = 0; i < NUM_NODES; i++){
		if(data->nodeID != nodeCache[i].nodeID){//ignore self	
			if((data->x_coor <= nodeCache[i].x_coor + 5) && (data->x_coor >= nodeCache[i].x_coor - 5) && (data->y_coor <= nodeCache[i].y_coor + 5) && (data->y_coor >= nodeCache[i].y_coor - 5)){
				nodesInRange[numNodesInRange].nodeID = nodeCache[i].nodeID;//in range
				nodesInRange[numNodesInRange].x_coor = nodeCache[i].x_coor;
				nodesInRange[numNodesInRange].y_coor = nodeCache[i].y_coor;
				numNodesInRange++;
			}
			else if((nodeCache[i].x_coor < 5 && data->x_coor < 5) && (nodeCache[i].y_coor > 95 && data->y_coor > 95) && (data->y_coor < (nodeCache[i].y_coor + 5))){
				nodesInRange[numNodesInRange].nodeID = nodeCache[i].nodeID;//in range
				nodesInRange[numNodesInRange].x_coor = nodeCache[i].x_coor;
				nodesInRange[numNodesInRange].y_coor = nodeCache[i].y_coor;
				numNodesInRange++;
			}
			else if((nodeCache[i].x_coor > 95 && data->x_coor > 95) && (nodeCache[i].y_coor < 5 && data->y_coor < 5) && (data->y_coor > (nodeCache[i].y_coor - 5))){
				nodesInRange[numNodesInRange].nodeID = nodeCache[i].nodeID;//in range
				nodesInRange[numNodesInRange].x_coor = nodeCache[i].x_coor;
				nodesInRange[numNodesInRange].y_coor = nodeCache[i].y_coor;
				numNodesInRange++;
			}
			else if((nodeCache[i].x_coor > 95 && data->x_coor > 95) && (nodeCache[i].y_coor > 95 && data->y_coor > 95) && (data->y_coor > (nodeCache[i].y_coor - 5))){
				nodesInRange[numNodesInRange].nodeID = nodeCache[i].nodeID;//in range
				nodesInRange[numNodesInRange].x_coor = nodeCache[i].x_coor;
				nodesInRange[numNodesInRange].y_coor = nodeCache[i].y_coor;
				numNodesInRange++;
			}
			else if((nodeCache[i].x_coor < 5 && data->x_coor < 5) && (nodeCache[i].y_coor < 5 && data->y_coor < 5) && (data->y_coor < (nodeCache[i].y_coor + 5))){
				nodesInRange[numNodesInRange].nodeID = nodeCache[i].nodeID;//in range
				nodesInRange[numNodesInRange].x_coor = nodeCache[i].x_coor;
				nodesInRange[numNodesInRange].y_coor = nodeCache[i].y_coor;
				numNodesInRange++;
			}
		}
	}
	
	/**************************************MAJORITY OF WORKLOAD**************************************/
	while(totalTime != EXP_DURATION){//main process that runs for exp_duration, total time is incremented every dwell_duration
		
		if(data->is_noisemaker == 1){
			if(((random() % 100) + 1) > data->dwell_probability_noisemakers){//decide whether to switch channels if noisemaker
				channel = random() % 3;
			}
		}else{
			if(((random() % 100) + 1) > data->dwell_probability){//decide whether to switch channels
				channel = random() % 3;
			}
		}
		
		/***********************************************Writing to Channels***********************************************/
		if(channel == 0){//channel 1
			usleep(data->talk_window_time);//simulating the consideration of sending a message
			if(data->is_noisemaker == 1){
				pthread_mutex_lock(&channel_1lock);
					usleep(data->dwell_noisemakers);//simulating noise, take over the channel by using a lock
				pthread_mutex_unlock(&channel_1lock);
			}
			else if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message to global channel
				messageData newMessage;
				strcpy(newMessage.transMess, mess[random() % 25]);	
				newMessage.nodeID = data->nodeID;
				sprintf(uniqueMess,"%d", newMessage.nodeID);
				strncat(newMessage.transMess, uniqueMess, strlen(uniqueMess) + strlen(newMessage.transMess));
				newMessage.channel = 1;
				newMessage.x_coor = data->x_coor;
				newMessage.y_coor = data->y_coor;
				pthread_mutex_lock(&channel_1lock);
					memcpy(&channel_1[channel_1count], &newMessage, sizeof(messageData));
					usleep(data->transmission_time);//simulating transmission time
					if(++channel_1count == CHANNEL_SIZE){
						channel_1count = 0;
					}
				pthread_mutex_unlock(&channel_1lock);    
			}else{//if not sending a message re-transmit seen messages
				pthread_mutex_lock(&channel_1lock);
					usleep(data->transmission_time);//simulating transmission time
					memcpy(&channel_1[channel_1count], &messageBuffer[messageBuffCount], sizeof(messageData));
					if(++channel_1count == CHANNEL_SIZE){
						channel_1count = 0;
					}
				pthread_mutex_unlock(&channel_1lock);
				if(--messageBuffCount == -1){
					messageBuffCount = 99;
				}
			}  
		}else if(channel == 1){//channel 6
			usleep(data->talk_window_time);//simulating the consideration of sending a message
			if(data->is_noisemaker == 1){
				pthread_mutex_lock(&channel_6lock);
					usleep(data->dwell_noisemakers);//simulating noise
				pthread_mutex_unlock(&channel_6lock);
			}
			else if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message to global channel
				messageData newMessage;
				strcpy(newMessage.transMess, mess[random() % 25]);
				newMessage.nodeID = data->nodeID;
				sprintf(uniqueMess,"%d", newMessage.nodeID);
				strncat(newMessage.transMess, uniqueMess, strlen(uniqueMess) + strlen(newMessage.transMess));
				newMessage.channel = 6;
				newMessage.x_coor = data->x_coor;
				newMessage.y_coor = data->y_coor;
				pthread_mutex_lock(&channel_6lock);
					memcpy(&channel_6[channel_6count], &newMessage, sizeof(messageData));
					usleep(data->transmission_time);//simulating transmission time
					if(++channel_6count == CHANNEL_SIZE){
						channel_6count = 0;
					}
				pthread_mutex_unlock(&channel_6lock);    
			}else{//if not sending a message re-transmit seen messages
				pthread_mutex_lock(&channel_6lock);
					usleep(data->transmission_time);//simulating transmission time
					memcpy(&channel_6[channel_6count], &messageBuffer[messageBuffCount], sizeof(messageData));
					if(++channel_6count == CHANNEL_SIZE){
						channel_6count = 0;
					}
				pthread_mutex_unlock(&channel_6lock);
				if(--messageBuffCount == -1){
					messageBuffCount = 99;
				}
			}
		}else if(channel == 2){//channel 11
			usleep(data->talk_window_time);//simulating the consideration of sending a message
			if(data->is_noisemaker == 1){
				pthread_mutex_lock(&channel_11lock);
					usleep(data->dwell_noisemakers);//simulating noise
				pthread_mutex_unlock(&channel_11lock);
			}
			else if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message to global channel
				messageData newMessage;
				strcpy(newMessage.transMess, mess[random() % 25]);
				newMessage.nodeID = data->nodeID;
				sprintf(uniqueMess,"%d", newMessage.nodeID);
				strncat(newMessage.transMess, uniqueMess, strlen(uniqueMess) + strlen(newMessage.transMess));
				newMessage.channel = 11;
				newMessage.x_coor = data->x_coor;
				newMessage.y_coor = data->y_coor;
				pthread_mutex_lock(&channel_11lock);
					memcpy(&channel_11[channel_11count], &newMessage, sizeof(messageData));
					usleep(data->transmission_time);//simulating transmission time
					if(++channel_11count == CHANNEL_SIZE){
						channel_11count = 0;
					}
				pthread_mutex_unlock(&channel_11lock);    
			}else{//if not sending a message re-transmit seen messages
				pthread_mutex_lock(&channel_11lock);
					usleep(data->transmission_time);//simulating transmission time
					memcpy(&channel_11[channel_11count], &messageBuffer[messageBuffCount], sizeof(messageData));
					if(++channel_11count == CHANNEL_SIZE){
						channel_11count = 0;
					}
				pthread_mutex_unlock(&channel_11lock);
				if(--messageBuffCount == -1){
					messageBuffCount = 99;
				}
			}
		}
		
		memset(uniqueMess, 0, sizeof(uniqueMess));//reset unique concatenated message
		
		/****************************************reading from each channel****************************************/ 
		localChannelCount1 = channel_1count;//channel 1
		for(i = 0; i < numNodesInRange; i++){//for number of nodes in range
			if(nodesInRange[i].nodeID != 0){//ignore IDs that are 0             
				for(j = 0; j < localChannelCount1; j++){
					pthread_mutex_lock(&channel_1lock);
					if((nodesInRange[i].nodeID == channel_1[j].nodeID) && (channel_1[j].transMess != NULL)){//if ID of message in channel matches node in range, and the channel has a message						
						for(k = 0; k < messageBuffCount; k++){//check if the message has been seen
							if(!strcmp(channel_1[j].transMess, messageBuffer[k].transMess)){//true if already have seen message
								sawMess_ch1 = 1;//set flag if message has been seen
							}
						}
						if(sawMess_ch1 != 1){//if haven't already seen message, take it
							memcpy(&messageBuffer[messageBuffCount], &channel_1[j], sizeof(messageData));   
							memcpy(&logBuffer[logBuffCount], &messageBuffer[messageBuffCount], sizeof(messageData));
							currentTime = time(NULL);
							strcpy(logBuffer[logBuffCount].time, ctime(&currentTime));//get recieved time of day
							logBuffCount++;
							if(--channel_1count == -1){//wrapping index
								channel_1count = CHANNEL_SIZE - 1;	
							}
							if(++messageBuffCount == MESSAGE_BUFF_SIZE){//wrapping index
								messageBuffCount = 0;
							}
						}		
						sawMess_ch1 = 0;	
					} 
					pthread_mutex_unlock(&channel_1lock);
				} 
			}
		}
		localChannelCount6 = channel_6count;//channel 6
		for(i = 0; i < numNodesInRange; i++){//for number of nodes in range
			if(nodesInRange[i].nodeID != 0){ //ignore IDs that are 0 
				for(j = 0; j < localChannelCount6; j++){
					pthread_mutex_lock(&channel_6lock);
					if((nodesInRange[i].nodeID == channel_6[j].nodeID) && (channel_6[j].transMess != NULL)){//if ID of message in channel matches node in range, and the channel has a message
						for(k = 0; k < messageBuffCount; k++){
							if(!strcmp(channel_6[j].transMess, messageBuffer[k].transMess)){//true if already have seen message
								sawMess_ch6 = 1;//set flag if message has been seen
							}
						}
						if(sawMess_ch6 != 1){//if haven't already seen message, take it
							memcpy(&messageBuffer[messageBuffCount], &channel_6[j], sizeof(messageData));   
							memcpy(&logBuffer[logBuffCount], &messageBuffer[messageBuffCount], sizeof(messageData));
							currentTime = time(NULL);
							strcpy(logBuffer[logBuffCount].time, ctime(&currentTime));//get recieved time of day
							logBuffCount++;
							if(--channel_6count == -1){//wrapping index
								channel_6count = CHANNEL_SIZE - 1;	
							}
							if(++messageBuffCount == MESSAGE_BUFF_SIZE){//wrapping index
								messageBuffCount = 0;
							}
						}		
						sawMess_ch6 = 0;
					} 
					pthread_mutex_unlock(&channel_6lock);
				}
			}
		}
		localChannelCount11 = channel_11count;//channel 11
		for(i = 0; i < numNodesInRange; i++){//for number of nodes in range
			if(nodesInRange[i].nodeID != 0){ //ignore IDs that are 0
				for(j = 0; j < localChannelCount11; j++){
					pthread_mutex_lock(&channel_11lock);
					if((nodesInRange[i].nodeID == channel_11[j].nodeID) && (channel_11[j].transMess != NULL)){
						for(k = 0; k < messageBuffCount; k++){//check if the message has been seen
							if(!strcmp(channel_11[j].transMess, messageBuffer[k].transMess)){//true if already have seen message
								sawMess_ch11 = 1;//set flag if message has been seen
							}
						}
						if(sawMess_ch11 != 1){//if haven't already seen message, take it
							memcpy(&messageBuffer[messageBuffCount], &channel_11[j], sizeof(messageData));   
							memcpy(&logBuffer[logBuffCount], &messageBuffer[messageBuffCount], sizeof(messageData));
							currentTime = time(NULL);
							strcpy(logBuffer[logBuffCount].time, ctime(&currentTime));//get recieved time of day
							logBuffCount++;
							if(--channel_11count == -1){//wrapping index
								channel_11count = CHANNEL_SIZE - 1;	
							}
							if(++messageBuffCount == MESSAGE_BUFF_SIZE){//wrapping index
								messageBuffCount = 0;
							}
						}		
						sawMess_ch11 = 0;					
					} 
					pthread_mutex_unlock(&channel_11lock);
				}
			}
		}

		usleep(data->dwell_duration);     
		totalTime++;
	}
	
	if(logBuffCount > 0){//print log file
		FILE* logFile;
		sprintf(filename, "%d", data->nodeID);
		strncat(filename, ".txt", sizeof(".txt"));
		logFile = fopen(filename, "a+");
		fprintf(logFile, "Log for node %d: with coordinates: x = %d, y = %d\n", data->nodeID, data->x_coor, data->y_coor);
		fprintf(logFile, "Nodes In Rage:\n");
		for(i = 0; i< numNodesInRange; i++){
			fprintf(logFile, "\t%d x = %d, y = %d\n", nodesInRange[i].nodeID, nodesInRange[i].x_coor, nodesInRange[i].y_coor);
		}
		fprintf(logFile, "Saw messages from:\n");
		for(i = 0; i < logBuffCount; i++){
			fprintf(logFile, "ID: %d\n \tMessage: %s\n \tx = %d\n \ty = %d\n \tChannel = %d\n \tTime Recieved: %s", logBuffer[i].nodeID, logBuffer[i].transMess, logBuffer[i].x_coor,\
											  															   		    logBuffer[i].y_coor, logBuffer[i].channel, logBuffer[i].time);
		}
		fclose(logFile);
	}

	pthread_exit(NULL);
}

int main(int argc, char** argv){
	srand(time(NULL));
	int i, checkError;

	pthread_t nodes[NUM_NODES];
	nodeData nodeInfo[NUM_NODES];//data struct for each thread

	pthread_mutex_init(&channel_1lock, NULL);
	pthread_mutex_init(&channel_6lock, NULL);
	pthread_mutex_init(&channel_11lock, NULL);

	if(argc == 1){
		puts("Running Simulation...");
		for(i = 0; i < NUM_NODES; i++){
			nodeInfo[i].nodeID = (random() % UINT_MAX) + 1;//set node Id, 4 bytes
			nodeCache[i].nodeID = nodeInfo[i].nodeID;//line stay for testing and non testing

			/*non-testing coordinate generation*/
			nodeInfo[i].x_coor = random() % 100;
			nodeInfo[i].y_coor = random() % 100;
			nodeCache[i].x_coor = nodeInfo[i].x_coor;
			nodeCache[i].y_coor = nodeInfo[i].y_coor;

			nodeInfo[i].dwell_duration = 1000000;
			nodeInfo[i].dwell_probability = 1;
			nodeInfo[i].transmission_time = 100;
			nodeInfo[i].talk_window_time = 100;
			nodeInfo[i].talk_probability = 50;
			if(i < NUM_NOISE_MAKERS){//make desired amount of nodes noisemakers
				nodeInfo[i].is_noisemaker = 1;
				nodeInfo[i].dwell_noisemakers = (random() % 2000) + 1000;
				nodeInfo[i].dwell_probability_noisemakers = 20;
			}else{
				nodeInfo[i].is_noisemaker = 0;
				nodeInfo[i].dwell_noisemakers = 0;
				nodeInfo[i].dwell_probability_noisemakers = 0;	
			}

			if((checkError = pthread_create(&nodes[i], NULL, nodeProcess, &nodeInfo[i]))){
				fprintf(stderr, "Failed to create thread with pthread_create().\n");
				return 0;
			}
		}

		for (i = 0; i < NUM_NODES; ++i) {
			pthread_join(nodes[i], NULL);
		}
		
		pthread_mutex_destroy(&channel_1lock);
		pthread_mutex_destroy(&channel_6lock);
		pthread_mutex_destroy(&channel_11lock);
		puts("Simulation Complete...");
	}else{
		puts("Correct usage: ./macmutex <no arguments>");
	}

	return 0;
}

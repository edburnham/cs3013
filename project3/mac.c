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

#define SEED_VALUE 10
#define NUM_NODES 2
#define NUM_NOISE_MAKERS 0

/*random messages to send*/
char* mess[15] = {"hello", "goodbye", "cs3013", "orange", "purple", "michael!", "edward!", "johnson!", "watson!", "danger!", "stranger!", "things!", "DOOM!", "highlander!", "whatThe!"};

sem_t channel_1lock;
sem_t channel_6lock;
sem_t channel_11lock;

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
int channel_1count = 0;
int channel_6count = 0;
int channel_11count = 0;

messageData nodeCache[NUM_NODES];

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
    double dwell_noisemakers;
    double dwell_probability_noisemakers;
}nodeData;

/*thread process*/
void* nodeProcess(void* nodeInfo){
    nodeData* data = (nodeData*)nodeInfo;
    int messageBuffCount = 0;
    int channel = random() % 2;
    useconds_t time = data->dwell_duration;
    
    typedef struct inRange{
	unsigned int nodeID;
	int x_coor;
	int y_coor;
    }inRange;
    
    messageData messageBuffer[100];
    
    inRange nodesInRange[100];
    //char* messageBuff[100];
    int i;
    printf("---- NODE: %d, @X: %d, Y: %d ----\n", data->nodeID, data->x_coor, data->y_coor);
    //printf("0 ID: %d\n1 ID: %d\n", nodeCache[0].nodeID, nodeCache[1].nodeID);
    for(i = 0; i < NUM_NODES; i++){
        if(data->nodeID != nodeCache[i].nodeID){	
            //printf("xcoor: %d ycoor: %d\n", nodeCache[i].x_coor, nodeCache[i].y_coor);
            if((data->x_coor <= nodeCache[i].x_coor + 5) && (data->x_coor >= nodeCache[i].x_coor - 5)&& (data->y_coor <= nodeCache[i].y_coor + 5) && (data->y_coor >= nodeCache[i].y_coor - 5)){
            nodesInRange[i].nodeID = nodeCache[i].nodeID;//in range
            nodesInRange[i].x_coor = nodeCache[i].x_coor;//in range
            nodesInRange[i].y_coor = nodeCache[i].y_coor;//in range
            }
            else if((nodeCache[i].x_coor < 5 && data->x_coor < 5) && (nodeCache[i].y_coor > 95 && data->y_coor > 95) && (data->y_coor < (nodeCache[i].y_coor + 5))){
            nodesInRange[i].nodeID = nodeCache[i].nodeID;//in range
            nodesInRange[i].x_coor = nodeCache[i].x_coor;//in range
            nodesInRange[i].y_coor = nodeCache[i].y_coor;//in range
            }
            else if((nodeCache[i].x_coor > 95 && data->x_coor > 95) && (nodeCache[i].y_coor < 5 && data->y_coor < 5) && (data->y_coor > (nodeCache[i].y_coor - 5))){
            nodesInRange[i].nodeID = nodeCache[i].nodeID;//in range
            nodesInRange[i].x_coor = nodeCache[i].x_coor;//in range
            nodesInRange[i].y_coor = nodeCache[i].y_coor;//in range
            }
            else if((nodeCache[i].x_coor > 95 && data->x_coor > 95) && (nodeCache[i].y_coor > 95 && data->y_coor > 95) && (data->y_coor > (nodeCache[i].y_coor - 5))){
            nodesInRange[i].nodeID = nodeCache[i].nodeID;//in range
            nodesInRange[i].x_coor = nodeCache[i].x_coor;//in range
            nodesInRange[i].y_coor = nodeCache[i].y_coor;//in range
            }
            else if((nodeCache[i].x_coor < 5 && data->x_coor < 5) && (nodeCache[i].y_coor < 5 && data->y_coor < 5) && (data->y_coor < (nodeCache[i].y_coor + 5))){
            nodesInRange[i].nodeID = nodeCache[i].nodeID;//in range
            nodesInRange[i].x_coor = nodeCache[i].x_coor;//in range
            nodesInRange[i].y_coor = nodeCache[i].y_coor;//in range
            }
        }
    }
	
    for(i = 0; i < NUM_NODES; i++){
        if(nodesInRange[i].nodeID != 0) {
            printf("Node in range: %d, x: %d, y: %d\n", nodesInRange[i].nodeID,nodesInRange[i].x_coor,nodesInRange[i].y_coor);
        }
    }

      while(1){
          /*dwell duration, transmission time and random switching*/
          if(((random() % 100) + 1) > data->dwell_probability){//decide whether to switch channels
              channel = random() % 2;
          }
          if(channel == 0){//channel 1
                  if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message to global channel
                        messageData newMessage;
                        newMessage.transMess = strdup(mess[rand() % 14]);
                        newMessage.nodeID = data->nodeID;
                        newMessage.channel = 1;
                        newMessage.x_coor = data->x_coor;
                        newMessage.y_coor = data->y_coor;
                        sem_wait(&channel_1lock);
                        memcpy(&channel_1[channel_1count], &newMessage, sizeof(messageData));
                        if(++channel_1count == 1000){
                           channel_1count = 0;
                        }
                        sem_post(&channel_1lock);    
                  }else{//local buffer retransmission
                      sem_wait(&channel_1lock);
                        memcpy(&channel_1[channel_1count], &messageBuffer[messageBuffCount], sizeof(messageData));
                        if(++channel_1count == 1000){
                           channel_1count = 0;
                        }
                      sem_post(&channel_1lock);
                      if(++messageBuffCount == 100){
                           messageBuffCount = 0;
                      }
                  }  
          }else if(channel  == 1){//channel 6
                if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message

                }
               if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message to global channel
                        messageData newMessage;
                        newMessage.transMess = strdup(mess[rand() % 14]);
                        newMessage.nodeID = data->nodeID;
                        newMessage.channel = 6;
                        newMessage.x_coor = data->x_coor;
                        newMessage.y_coor = data->y_coor;
                        sem_wait(&channel_6lock);
                        memcpy(&channel_6[channel_6count], &newMessage, sizeof(messageData));
                        if(++channel_1count == 1000){
                           channel_1count = 0;
                        }
                        sem_post(&channel_6lock);    
                  }else{//local buffer retransmission
                      sem_wait(&channel_6lock);
                        memcpy(&channel_6[channel_6count], &messageBuffer[messageBuffCount], sizeof(messageData));
                        if(++channel_6count == 1000){
                           channel_6count = 0;
                        }
                      sem_post(&channel_6lock);
                      if(++messageBuffCount == 100){
                           messageBuffCount = 0;
                      }
                  }
          }else if(channel == 2){//channel 11
                if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message
                    
                }
               if(((random() % 100) + 1) < data->talk_probability){//decide whether to send message to global channel
                        messageData newMessage;
                        newMessage.transMess = strdup(mess[rand() % 14]);
                        newMessage.nodeID = data->nodeID;
                        newMessage.channel = 11;
                        newMessage.x_coor = data->x_coor;
                        newMessage.y_coor = data->y_coor;
                        sem_wait(&channel_11lock);
                        memcpy(&channel_11[channel_11count], &newMessage, sizeof(messageData));
                        if(++channel_11count == 1000){
                           channel_11count = 0;
                        }
                        sem_post(&channel_11lock);    
                  }else{//local buffer retransmission
                      sem_wait(&channel_11lock);
                        memcpy(&channel_11[channel_11count], &messageBuffer[messageBuffCount], sizeof(messageData));
                        if(++channel_11count == 1000){
                           channel_11count = 0;
                        }
                      sem_post(&channel_11lock);
                      if(++messageBuffCount == 100){
                           messageBuffCount = 0;
                      }
                  }
          }
              
        for(i = 0; i < channel_1count; i++){
            if(channel_1[i].nodeID == ){
                
            }
        }
        for(i = 0; i < channel_6count; i++){
            
        }
        for(i = 0; i < channel_11count; i++){
            
        }
		usleep(time);//transmission time
       
          
      }
	
    pthread_exit(NULL);
}

int main(int argc, char** argv){
    srand(time(NULL));
    int i, checkError;
	
    pthread_t nodes[NUM_NODES];
    nodeData nodeInfo[NUM_NODES];//data struct for each thread
    
	sem_init(&channel_1lock, 0, 1);
    sem_init(&channel_6lock, 0, 1);
    sem_init(&channel_11lock, 0, 1);
    
    if(argc == 1){
	for(i = 0; i < NUM_NODES; i++){
	    nodeInfo[i].nodeID = random() % UINT_MAX;//set node Id, 4 bytes
	    //nodeInfo[i].nodeID = i;//for testing
	    nodeCache[i].nodeID = nodeInfo[i].nodeID;//line stay for testing and non testing
			
	    /*non-testing coordinate generation*/
	    nodeInfo[i].x_coor = random() % 99;
	    nodeInfo[i].y_coor = random() % 99;
	    nodeCache[i].x_coor = nodeInfo[i].x_coor;
	    nodeCache[i].y_coor = nodeInfo[i].y_coor;
			
	    /*testing mode coordinate generation
	      nodeInfo[i].x_coor = 95 + 5*i ;
	      nodeInfo[i].y_coor = 5*i; 
	      nodeCache[i].x_coor = nodeInfo[i].x_coor;
	      nodeCache[i].y_coor = nodeInfo[i].y_coor;*/
			
	    nodeInfo[i].dwell_duration = 1000000;
	    nodeInfo[i].dwell_probability = 50;
	    nodeInfo[i].transmission_time = 1000000;
	    nodeInfo[i].talk_window_time = 1000000;
	    nodeInfo[i].talk_probability = 50;
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

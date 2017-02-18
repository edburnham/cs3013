//Edward Burnham
//Michael Caldwell

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SIZE 64

char memory[SIZE];
int freeList[4];
int hardwareReg[4];

int checkFreeList(int pageTable){
    int i;
    int ptLoc = 0;
    
    if (pageTable) {
	for(i = 0; i < 4; i++){
	    if(freeList[i] == 0){
		freeList[i] = 1;
		ptLoc = i;
		for(i = 0; i < 4; i++){
		    if(freeList[i] == 0){
			freeList[i] = 1;
			return ptLoc;
		    }
		}
	    }	    
	}
    }
    else {
	for(i = 0; i < 4; i++){
	    if(freeList[i] == 0){
		freeList[i] = 1;
		return i;
	    }
	}
    }
    
    return 0;//zero not a frame number
}

int main(void){
    char input[20];
    char temp[6];
    char pid[1];
    char instr_type[6];
    char virt_addr[6];
    char value[6];
	
    char pageTable[16];
	
    int PID;
    int i;
    int a = 0;
    int next = 0;
    int type = 0;
    int openPFN = 0;
    int length = 0;
    int countPID[4] = {0};
	
    memset(memory, 0, sizeof(memory));
    memset(freeList, 0, sizeof(freeList));
    memset(hardwareReg, 0, sizeof(hardwareReg));
	
    while(1){
	memset(input, 0, sizeof(input));
	memset(temp, 0, sizeof(temp));
	memset(pid, 0, sizeof(pid));
	memset(instr_type, 0, sizeof(instr_type));
	memset(virt_addr, 0, sizeof(virt_addr));
	memset(value, 0, sizeof(value));
		
	printf("Instruction? ");
	fgets(input, sizeof(input), stdin);
	length = strlen(input);
	input[length - 1] = '\0';
		
	if(length < 14){
	    //input[length + 1] = '\0';
	    /*Parse input and check for error on input values*/
	    for(i = 0; i < length + 1; i++){
		if(input[i] == ',' || input[i] == '\0'){
		    if(next == 0){//get pid
			strcpy(pid, temp);
			if(atoi(pid) > 3){
			    puts("PID must be less than 4.");
			    return 0;
			}
			next++;
			memset(temp, 0, sizeof(temp));
			a = 0;
		    }else if(next == 1){//get instruction type
			strcpy(instr_type, temp);
			if((strncmp("map", instr_type, strlen(instr_type)) != 0) && (strncmp("store", instr_type, strlen(instr_type)) != 0) && (strncmp("load", instr_type, strlen(instr_type)) != 0)){
			    puts("Only accepts map, store, and load for instruction type.");
			    return 0;
			}
			if(strncmp("map", instr_type, strlen(instr_type)) == 0){
			    type = 1;
			}else if(strncmp("store", instr_type, strlen(instr_type)) == 0){
			    type = 2;
			}else if(strncmp("load", instr_type, strlen(instr_type)) == 0){
			    type = 3;
			}
			next++;
			memset(temp, 0, sizeof(temp));
			a = 0;
		    }else if(next == 2){//get virtual address
			strcpy(virt_addr, temp);
			if(atoi(virt_addr) > 63){
			    puts("Virutal address must be 63 or less.");
			    return 0;
			}
			next++;
			memset(temp, 0, sizeof(temp));
			a = 0;
		    }else if(next == 3){//get value
			strcpy(value, temp);
			if(type == 1){
			    if(atoi(value) != 0 && atoi(value) != 1){
				puts("When using map, values must be 0 or 1.");
				return 0;
			    }
			}else if(type == 2){
			    if(atoi(value) > 255){
				puts("When using store, values must be 255 or less.");
				return 0;
			    }
			}else if(type == 3){
			    if(strncmp("NA", value, strlen(value)) != 0){
				puts("Value must be NA when using load.");
				return 0;
			    }
			}
			next++;
			memset(temp, 0, sizeof(temp));
			a = 0;
		    }
		}else{
		    temp[a] = input[i];
		    a++;
		}
	    }
			
	    /*************************************************Begin executing commands*************************************************/	
	    PID = atoi(pid);
	    if(strncmp("map", instr_type, strlen(instr_type)) == 0){
		if(hardwareReg[PID] != 0){
		    //printf("ERROR: virtual page already mapped into physical frame %d", hardwareReg[atoi(pid)]);
		    openPFN = checkFreeList(0); // passing 0 to indicate not creating page table
		    if(openPFN != 0){
			hardwareReg[PID] = openPFN;
			pageTable[0+4*countPID[PID]] = (char)PID;//PID
			pageTable[1+4*countPID[PID]] = (char)openPFN;//PFN
			pageTable[2+4*countPID[PID]] = (char)value; //R/W bit
			memcpy(&memory[] , &pageTable, sizeof(pageTable));
			countPID[PID]++;
		    }else{
			puts("Physical memory full");
		    }   
		    printf("Mapped virtual address %s (page %d) into physical frame %d", virt_addr, countPID[PID], openPFN);
		
		}else{
					
		    openPFN = checkFreeList(1); // passing 1 to indicate we're creating a page table
		    if(openPFN != 0){
			hardwareReg[PID] = openPFN;
			pageTable[0] = (char)PID;//PID
			pageTable[1] = (char)openPFN;//PFN
			pageTable[2] = (char)value; //R/W bit
			memcpy(&memory[16 * (openPFN - 1)] , &pageTable, sizeof(pageTable));
			countPID[PID]++;
			
		    }else{
			puts("Physical memory full");
		    }
		    //where page table resides (frame number 1 - 4)
		    printf("Put page table for PID %d into physical frame %d", atoi(pid), hardwareReg[PID]);
		    printf("Mapped virtual address %s (page %d) into physical frame %d", virt_addr, countPID[PID], openPFN);
		}



	    }

	    //printf("%s %s %s %s\n", pid, instr_type, virt_addr, value);
		
		
	}else{
	    puts("Correct Usage: ./vmem <process_id,instruction_type,virtual_address,value>");
	    return 0;
	}
    }
	
    
    return 0;
}

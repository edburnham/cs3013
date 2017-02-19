//Edward Burnham
//Michael Caldwell

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SIZE 64

char memory[SIZE];

char pid[1];
char instr_type[6];
char virt_addr[6];
char value[6];

int freeList[4];
int hardwareReg[4];
int openPFNPage = 0;
int openPFNTable = 0; 
int countPID[4] = {0};
int vFreeList[4][4];

int checkFreeList(){
    int i;
    for(i = 0; i < 4; i++){
	if(freeList[i] == 0){
	    freeList[i] = 1;
	    return i; // returns PFN 1,2,3,4
	}
    }  
    return -1;// no free frames
}

void map(int PID, int VPN, char R_W){
	int ptOffset = 0;
	if(hardwareReg[PID] != -1){
		openPFNPage = checkFreeList(); // passing 0 to indicate not creating page table
		if(openPFNPage != -1){

			if (vFreeList[PID][VPN] != 1){
				vFreeList[PID][VPN] = 1;

				ptOffset = countPID[PID]*4;
				memory[(hardwareReg[PID])*16 + ptOffset] = (char)PID;//PID    /update page table
				ptOffset++;
				memory[(hardwareReg[PID])*16 + ptOffset] = (char)VPN;
				ptOffset++;
				memory[(hardwareReg[PID])*16 + ptOffset] = (char)openPFNPage;//PFN
				ptOffset++;
				memory[(hardwareReg[PID])*16 + ptOffset] = R_W; //R/W bit
				countPID[PID]++;
				printf("Mapped virtual address %d (page %d) into physical frame %d\n", atoi(virt_addr), countPID[PID], openPFNPage);
			}
			else {
				printf("Error: virtual page already mapped into physical frame %d.\n", memory[(hardwareReg[PID])*16 + 2]);
			}

		}else{
		puts("Physical memory full");
		}   

	}else{		
		openPFNTable = checkFreeList(); // passing 1 to indicate we're creating a page table
		openPFNPage = checkFreeList();
		if ((openPFNPage == -1) && (openPFNTable == -1)){
		puts("Physical memory full");
		}
		else {
			hardwareReg[PID] = openPFNTable;			
			if (vFreeList[PID][VPN] != 1){
				vFreeList[PID][VPN] = 1;
				memory[(hardwareReg[PID])*16] = (char)PID;//PID
				memory[(hardwareReg[PID])*16 + 1] = (char)VPN;//PID
				memory[(hardwareReg[PID])*16 + 2] = (char)openPFNPage;//PFN
				memory[(hardwareReg[PID])*16 + 3] = R_W; //R/W bit
				countPID[PID]++;
				printf("Put page table for PID %d into physical frame %d\n", PID, openPFNTable);
				printf("Mapped virtual address %d (page %d) into physical frame %d\n", atoi(virt_addr), VPN, openPFNPage);

			}
			else {

				printf("Error: virtual page already mapped into physical frame %d.\n", memory[(hardwareReg[PID])*16 + 2]);
			}				
		}

	}
}
	
int store(int PID, int virt_addr, int val){
	int VPN = virt_addr/16;
	int PFN = 0;
	int offset = virt_addr - VPN*16;
	int i = 0;
	
	for(i = 0; i < 4; i++){
		if((int)memory[(hardwareReg[PID])*16 + i*4 + 1] == VPN){
			PFN = memory[(hardwareReg[PID])*16 + i*4 + 2];
			memory[PFN*16 + offset] = (char)val;
			printf("Stored value %d at virtual address %d (physical address %d)\n", val, virt_addr, PFN*16 + offset);
			return 1;
		}
	}
	return 0;
}

int load(int PID, int virt_addr){
	int VPN = virt_addr/16;
	int PFN = 0;
	int offset = virt_addr - VPN*16;
	int value = 0;
	int i = 0;
	
	for(i = 0; i < 4; i++){
		if((int)memory[(hardwareReg[PID])*16 + i*4 + 1] == VPN){
			PFN = memory[(hardwareReg[PID])*16 + i*4 + 2];
			value = memory[PFN*16 + offset];
			printf("The value %d is virtual address %d (physical address %d)\n", value, virt_addr, PFN*16 + offset);
			return 1;
		}
	}
	return 0;
}


int main(void){
    char input[20];
    char temp[6];
   
		
    int PID = 0;
    int i;
    int a = 0;
    int next = 0;
    int type = 0;
	int length = 0;
    int val = 0;
    int VPN = 0;
	int address = 0;
    
    memset(vFreeList, 0, sizeof(vFreeList));
    memset(countPID, 0, sizeof(countPID));	
    memset(memory, 0, sizeof(memory));
    memset(freeList, 0, sizeof(freeList));
    memset(hardwareReg, -1, sizeof(hardwareReg));
	
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
		next = 0;
		a = 0;
		
		if(strncmp("\0", input, 1) == 0){
			return 0;
		}
		
	    /*Parse input and check for error on input values*/
	    for(i = 0; i < length + 1; i++){
		if(input[i] == ',' || input[i] == '\0'){
		    if(next == 0){//get pid
			strcpy(pid, temp);
			if(atoi(pid) > 3){
			    puts("PID must be less than 4.");
			}
			next++;
			memset(temp, 0, sizeof(temp));
			a = 0;
			//printf("HERE1: %s", pid);
		    }else if(next == 1){//get instruction type
			strcpy(instr_type, temp);
			if((strncmp("map", instr_type, strlen(instr_type)) != 0) && (strncmp("store", instr_type, strlen(instr_type)) != 0) && (strncmp("load", instr_type, strlen(instr_type)) != 0)){
			    puts("Only accepts map, store, and load for instruction type.");
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
			}
			next++;
			memset(temp, 0, sizeof(temp));
			a = 0;
		    }else if(next == 3){//get value
			strcpy(value, temp);
				if(type == 1){/*************************************************Begin executing commands*************************************************/
					/*map*/
					if(atoi(value) != 0 && atoi(value) != 1){
						puts("When using map, values must be 0 or 1.");
					}else{
						PID = atoi(pid);
						VPN = atoi(virt_addr)/16;
						map(PID, VPN, value[0]);
					}
				}else if(type == 2){
					/*store*/
					if(atoi(value) > 255){
						puts("When using store, values must be 255 or less.");
					}else{
						PID = atoi(pid);
						address = atoi(virt_addr);
						val = atoi(value);
						if(store(PID, address, val) == 0){
							puts("store failed!");	
						}
					}
					
				}else if(type == 3){
					/*load*/
					if(strncmp("NA", value, strlen(value)) != 0){
						puts("Value must be NA when using load.");
					}else{
						PID = atoi(pid);
						address = atoi(virt_addr);
						if(load(PID, address) != 0){
							puts("load Failed!");	
						}
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
	}
	     
    return 0;
}

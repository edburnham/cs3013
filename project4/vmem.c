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

int freeList[4] = {0};;
unsigned long hardwareReg[4] = {0};
int openPFNPage = 0;
int openPFNTable = 0; 
int countPID[4] = {0};
int vFreeList[4][4] = {{0}, {0}};
int RR_PIDeviction = 0;
int firstSwap = 0;
unsigned long diskLen = 0;

FILE* hardDisk;

int PIDSwap(int PID) {//not evicting PID passed in, evicting next PID in RR, evicting all pages and page table at the same time
    int i;
	int tempF = 0;		
	
	if((int)memory[hardwareReg[RR_PIDeviction]*16] != PID){//if the PID is not our own
		hardDisk = fopen("hardDisk.txt", "a+");//evict full page table
		fprintf(hardDisk, "#%d\n", (int)memory[hardwareReg[RR_PIDeviction]*16]);//PID
		for(i = 1; i < 16; i++){
			fprintf(hardDisk, "%d\n", (int)memory[hardwareReg[RR_PIDeviction]*16 + i]);//VPN
			diskLen++;
		}		
		
		tempF = (int)memory[hardwareReg[RR_PIDeviction]*16 + 2];
		for(i = 0; i < 4; i++){	
			if(tempF == -1){
				fprintf(hardDisk, "%d\n", 0);
				diskLen++;
			}else{
				fprintf(hardDisk, "%d\n", (int)memory[tempF*16 + i]);
				diskLen++;
			}
		}
		tempF = (int)memory[hardwareReg[RR_PIDeviction]*16 + 6];
		for(i = 4; i < 8; i++){	
			if(tempF == -1){
				fprintf(hardDisk, "%d\n", 0);
				diskLen++;
			}else{
				fprintf(hardDisk, "%d\n", (int)memory[tempF*16 + i]);
				diskLen++;
			}
		}
		tempF = (int)memory[hardwareReg[RR_PIDeviction]*16 + 10];
		for(i = 8; i < 12; i++){	
			if(tempF == -1){
				fprintf(hardDisk, "%d\n", 0);
				diskLen++;
			}else{
				fprintf(hardDisk, "%d\n", (int)memory[tempF*16 + i]);
				diskLen++;
			}
		}
		tempF = (int)memory[hardwareReg[RR_PIDeviction]*16 + 14];
		for(i = 12; i < 16; i++){	
			if(tempF == -1){
				fprintf(hardDisk, "%d\n", 0);
				diskLen++;
			}else{
				fprintf(hardDisk, "%d\n", (int)memory[tempF*16 + i]);
				diskLen++;
			}
		}
		fclose(hardDisk);
		freeList[hardwareReg[RR_PIDeviction]] = 0;
		freeList[tempF] = 0;
		if(diskLen == 0){
			firstSwap = 1;
			puts("made it");
			hardwareReg[RR_PIDeviction] = 4;
			return 1;
		}
		hardwareReg[RR_PIDeviction] = diskLen;
		return 1;
	}
	
	fclose(hardDisk);
	
	if(++RR_PIDeviction == 4){
		RR_PIDeviction = 0;	
	}

	return 0;
  	
}

int swapIn(int PID, int VPN, int openPFNTable, int openPFNPage){
	int c;
	int i;
	int lineMultiplier = 0;
	int j = 0;
	char diskLines[10];
	
	if(firstSwap){
		lineMultiplier = 16;
	}else{
		lineMultiplier = hardwareReg[PID];
	}
	
	hardDisk = fopen("hardDisk.txt", "r");		
		for(i = 0; i < lineMultiplier; i++){//get page table
			c = fgetc(hardDisk);
		    while(c != '\n'){
				diskLines[j] = c;
				j++;
				c = fgetc(hardDisk);
			}
			if(i == (VPN*4 + 2)){
				memory[openPFNTable*16 + i] = (char)openPFNPage;
			}else{
				memory[openPFNTable*16 + i] = (char)atoi(diskLines);
				j = 0;
			}		
			memset(diskLines, 0 ,sizeof(diskLines));
		}	
		
		j = 0;
		
		for(i = VPN*4; i < VPN*4 + 4; i++){
			c = fgetc(hardDisk);
		    while(c != '\n'){
				diskLines[j] = c;
				j++;
				c = fgetc(hardDisk);
			}
			memory[openPFNPage*16 + i] = (char)atoi(diskLines);
			j = 0;
		}
		
		hardwareReg[PID] = openPFNTable;			
		fclose(hardDisk);
	return 0;
}

int checkFreeList(){
    int i;
    for(i = 0; i < 4; i++){
		if(freeList[i] == 0){
			freeList[i] = 1;
			return i; // returns PFN 0,1,2,3
		}
    }  
    return -1;// no free frames
}

void map(int PID, int VPN, int R_W){
    int ptOffset = 0;
	
    if(hardwareReg[PID] != -1){//adding a page to an existing page table
		openPFNPage = checkFreeList(); 
		if(openPFNPage != -1){
			while(PIDSwap(PID) != 1);
			openPFNPage = checkFreeList();
			puts("Swapping");
		}	
	    if (vFreeList[PID][VPN] != 1){
			vFreeList[PID][VPN] = 1;
			ptOffset = countPID[PID]*4;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)PID;//PID    
			ptOffset++;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)VPN;
			ptOffset++;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)openPFNPage;//PFN
			ptOffset++;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)R_W; //R/W bit
			countPID[PID]++;
			printf("Mapped virtual address %d (page %d) into physical frame %d\n", atoi(virt_addr), countPID[PID], openPFNPage);
	    }
	    else {
			printf("Error: virtual page already mapped into physical frame %d.\n", memory[(hardwareReg[PID])*16 + 2]);
	    }

	}else{	//create page table	
		openPFNTable = checkFreeList(); 
		openPFNPage = checkFreeList();
		if ((openPFNPage == -1) && (openPFNTable == -1)){
			while(PIDSwap(PID) != 1);
			openPFNTable = checkFreeList(); 
			openPFNPage = checkFreeList();
			puts("Swapping");
		}
		
			hardwareReg[PID] = openPFNTable;			
			if (vFreeList[PID][VPN] != 1){
				vFreeList[PID][VPN] = 1;
				memory[hardwareReg[PID]*16] = (char)PID;//PID
				memory[hardwareReg[PID]*16 + 1] = (char)VPN;//VPN for page table
				memory[hardwareReg[PID]*16 + 2] = (char)openPFNPage;//PFN
				memory[hardwareReg[PID]*16 + 3] = (char)R_W; //R/W bit

				memory[hardwareReg[PID]*16 + 6] = (char)(-1);//
				memory[hardwareReg[PID]*16 + 10] = (char)(-1);//
				memory[hardwareReg[PID]*16 + 14] = (char)(-1);//
				
				countPID[PID]++;
				printf("Put page table for PID %d into physical frame %d\n", PID, openPFNTable);
				printf("Mapped virtual address %d (page %d) into physical frame %d\n", atoi(virt_addr), VPN, openPFNPage);
			}
			else {
				printf("Error: virtual page already mapped into physical frame %d.\n", memory[(hardwareReg[PID])*16 + 2]);
			}				
    }
}
	
int store(int PID, int virt_addr, int val){
    int VPN = virt_addr/16;
    int PFN = 0;
    int offset = virt_addr - VPN*16;
    int R_W = 0;
    int i = 0;
	int openPFNTable = 0;
	int openPFNPage = 0;
      
	if(hardwareReg[PID] > 3){
		if(firstSwap == 1){
			openPFNTable = checkFreeList();
			openPFNPage = checkFreeList();
			if(openPFNTable == -1 && openPFNPage == -1){
				while(PIDSwap(PID) != 1);
				openPFNTable = checkFreeList();
				openPFNPage = checkFreeList();
			}
			swapIn(PID, VPN, openPFNTable, openPFNPage);
			firstSwap = 0;
		}
	}
    for(i = 0; i < 4; i++){
		if((int)memory[hardwareReg[PID]*16 + i*4 + 1] == VPN){
			PFN = memory[hardwareReg[PID]*16 + i*4 + 2];
			R_W = memory[hardwareReg[PID]*16 + i*4 + 3];
			if(R_W == 0){
				return -1;
			}else{
				memory[PFN*16 + offset] = (char)val;//store
				printf("Stored value %d at virtual address %d (physical address %d)\n", val, virt_addr, PFN*16 + offset);
				return 1;
			}
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
    int storeRetVal = 0;
	int rw = 0;
	
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
		
	if(strncmp("\0", input, 1) == 0){//reached end of file
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
				rw = atoi(value);
			    map(PID, VPN, rw);
			}
		    }else if(type == 2){
			/*store*/
			if(atoi(value) > 255){
			    puts("When using store, values must be 255 or less.");
			}else{
			    PID = atoi(pid);
			    address = atoi(virt_addr);
			    val = atoi(value);
			    storeRetVal = store(PID, address, val);
			    if(storeRetVal == 0){
				puts("store failed!");	
			    }else if(storeRetVal == -1){
				puts("permissions only allow reading!");
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

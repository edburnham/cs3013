//Edward Burnham
//Michael Caldwell

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define SIZE 64

char memory[SIZE];

char pid[1];
char instr_type[6];
char virt_addr[6];
char value[6];

unsigned long hardwareReg[4] = {0, 0, 0, 0};
int diskLen = 0;

int freeList[4] = {0, 0 , 0, 0};
int openPFNPage = 0;
int openPFNTable = 0; 
int countPID[4] = {0, 0, 0, 0};
int vFreeList[4][4] = {{0}, {0}};
int RR_PIDeviction = 0;
int firstSwap = 0;


FILE* hardDisk;

void swap(int PID) {// Done - untested
	int i;
	int isPageTable = 0;
	int temp = 0;
	int temp2 = 0;

	if (RR_PIDeviction == hardwareReg[PID]){ //if its our own PID, skip it.
		if(++RR_PIDeviction == 4){
			RR_PIDeviction = 0;	
		}
	}
	for(i = 0; i < 4; i++){
		if (hardwareReg[i] == RR_PIDeviction && i != PID){
			isPageTable = 1;
			temp2 = i;
			break;
		}
	}
	if (isPageTable){
		temp = hardwareReg[temp2];
		hardwareReg[temp2] = diskLen + 4; 
		hardDisk = fopen("hardDisk.txt", "a+");//evict full page table
		fwrite(&memory[temp*16], sizeof(char), 1, hardDisk);//PID
		diskLen++;
		for(i = 1; i < 16; i++){
			fwrite(&memory[temp*16 + i], sizeof(char), 1, hardDisk);
			diskLen++;
		}		
		fclose(hardDisk);
		printf("Swapped frame %u to disk at offset %d.\n", RR_PIDeviction, diskLen - 16);
		freeList[PID] = 0;
	}
	else{
		for(i = 0; i < 4; i++){
			if(hardwareReg[i] != -1){
				if(memory[hardwareReg[i]*16 + 2]  == RR_PIDeviction){
					memory[hardwareReg[i]*16 + 2] = diskLen + 4;
				}
			}
		}
		
		hardDisk = fopen("hardDisk.txt", "a+");
		fwrite(&memory[RR_PIDeviction*16], sizeof(char), 1, hardDisk);//PID
		diskLen++;
		for(i = 1; i < 16; i++){
			fwrite(&memory[RR_PIDeviction*16 + i], sizeof(char), 1, hardDisk);
			diskLen++;
		}		
		fclose(hardDisk);
		printf("Swapped frame %u to disk at offset %d.\n", RR_PIDeviction, diskLen - 16);
		freeList[RR_PIDeviction] = 0;
	}
	if(++RR_PIDeviction == 4){
		RR_PIDeviction = 0;	
	}
}

void swapIn(int PID, int VPN, int openPFN, int isTable){
	int diskIndex = 0;
	char diskLines[16] = {0};

	if(isTable) { //It's a page table!
		diskIndex = hardwareReg[PID] - 4;
		hardDisk = fopen("hardDisk.txt", "r");	
		if(fseek(hardDisk, diskIndex, SEEK_SET) < 0){
			puts("lseek failed");
		}
		fread(&diskLines, sizeof(char), 16, hardDisk);
		memcpy(&memory[openPFN*16], &diskLines, sizeof(diskLines));
		freeList[openPFN] = 1;
		hardwareReg[PID] = openPFN;			
		fclose(hardDisk);

	}
	else { // it's just a page
		diskIndex = memory[hardwareReg[PID]*16 + (VPN*4 + 2)] - 4; // grab the diskIndex of the page from the page table located in the PFN field
		memory[hardwareReg[PID]*16 + (VPN*4 + 2)] = openPFN; // set the new pfn in the page table.
		hardDisk = fopen("hardDisk.txt", "r");
		
		if(fseek(hardDisk, diskIndex, SEEK_SET) < 0){
			puts("lseek failed");
		}

		//for(i = 0; i < 16; i++){
			fread(diskLines, sizeof(char), 16, hardDisk);
		//		puts("freqding");	
		//	}
			//printf("%c", diskLines[i]);
		//}
		
		//printf("lines: %s\n", diskLines);
		memcpy(&memory[openPFN*16], &diskLines, sizeof(diskLines));
		freeList[openPFN] = 1;
		printf("Swapped disk offset %d into frame %d.\n", diskIndex, openPFN);		
		fclose(hardDisk);
	}
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

void map(int PID, int VPN, int R_W){ // NEEDS FIXIN
	int ptOffset = 0;

	if(hardwareReg[PID] != -1){//adding a page to an existing page table
		if (vFreeList[PID][VPN] != 1){
			vFreeList[PID][VPN] = 1;
			openPFNPage = checkFreeList(); 
			//printf("open: %d\n", openPFNTable);
			if(openPFNPage == -1){
				swap(PID);
				openPFNPage = checkFreeList();
			}	
			ptOffset = VPN*4;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)PID;//PID    
			ptOffset++;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)VPN;
			ptOffset++;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)openPFNPage;//PFN
			ptOffset++;
			memory[hardwareReg[PID]*16 + ptOffset] = (char)R_W; //R/W bit
			countPID[PID]++;
			printf("Mapped virtual address %d (page %d) into physical frame %d.\n", atoi(virt_addr), VPN, openPFNPage);
		}
		else {
			printf("Error: virtual page already mapped into physical frame %d.\n", memory[(hardwareReg[PID])*16 + 2]);
		}

	}else{	//create page table --- THIS IS FUCKED NEED TO REVISIT	
		if (vFreeList[PID][VPN] != 1){
			vFreeList[PID][VPN] = 1;
			openPFNTable = checkFreeList(); 
			//printf("open: %d\n", openPFNTable);
			if(openPFNTable == -1){
				swap(PID);
				openPFNTable = checkFreeList();
			}
			openPFNPage = checkFreeList();
			//printf("open: %d\n", openPFNPage);
			if(openPFNPage == -1){
				swap(PID);
				openPFNPage = checkFreeList();
			}

			hardwareReg[PID] = openPFNTable;			
			memory[hardwareReg[PID]*16] = (char)PID;//PID
			memory[hardwareReg[PID]*16 + 1] = (char)VPN;//VPN for page table
			memory[hardwareReg[PID]*16 + 2] = (char)openPFNPage;//PFN
			memory[hardwareReg[PID]*16 + 3] = (char)R_W; //R/W bit

			memory[hardwareReg[PID]*16 + 6] = (char)(-1);//
			memory[hardwareReg[PID]*16 + 10] = (char)(-1);//
			memory[hardwareReg[PID]*16 + 14] = (char)(-1);//

			countPID[PID]++;
			printf("Put page table for PID %d into physical frame %d.\n", PID, openPFNTable);
			printf("Mapped virtual address %d (page %d) into physical frame %d.\n", atoi(virt_addr), VPN, openPFNPage);
		}
		else {
			printf("Error: virtual page already mapped into physical frame %d.\n", memory[(hardwareReg[PID])*16 + 2]);
		}				
	}
}

int store(int PID, int virt_addr, int val){ // NEEDS FIXIN
	int VPN = virt_addr/16;
	int offset = virt_addr - VPN*16;
	int PFN = 0;
	int R_W = 0;

	if(hardwareReg[PID] > 3){
		//this means page table is on disk, need to swap it in.
		openPFNTable = checkFreeList();
		if (openPFNTable == -1){
			swap(PID);
			openPFNTable = checkFreeList();
		}
		swapIn(PID, VPN, openPFNTable, 1); // swapping in a table.
		// Then, we still need to swap the page
		openPFNPage = checkFreeList();
		if (openPFNPage == -1){
			swap(PID); // swappin out a page
			openPFNPage = checkFreeList();
		}
		//printf("2: %d\n" ,VPN);
		swapIn(PID,VPN,openPFNPage,0); // swappin in the page
		PFN = memory[(openPFNPage*16) + VPN*4 + 2];
		R_W = memory[(openPFNPage*16) + VPN*4 + 3];
		if(R_W == 0){
			puts("Error: writes are not allowed to this page");
			return -1;
		}else{
			memory[PFN*16 + offset] = (char)val;//store
			printf("Stored value %d at virtual address %d (physical address %d)\n", val, virt_addr, PFN*16 + offset);
			return 1;
		}
	}
	else if(memory[(hardwareReg[PID]*16) + VPN*4 + 2] > 3){
		// we just need to swap in a page
		openPFNPage = checkFreeList();
		if (openPFNPage == -1){
			swap(PID); // swappin out a page
			openPFNPage = checkFreeList();
		}

		swapIn(PID,VPN,openPFNPage,0); // swapping da page
		PFN = memory[(openPFNPage*16) + VPN*4 + 2];
		R_W = memory[(openPFNPage*16) + VPN*4 + 3];
		if(R_W == 0){
			puts("Error: writes are not allowed to this page");
			return -1;
		}else{
			memory[PFN*16 + offset] = (char)val;//store
			printf("Stored value %d at virtual address %d (physical address %d)\n", val, virt_addr, PFN*16 + offset);
			return 1;
		}
	}else{
		PFN = memory[(hardwareReg[PID]*16) + VPN*4 + 2];
		R_W = memory[(hardwareReg[PID]*16) + VPN*4 + 3];

		if(R_W == 0){
			puts("Error: writes are not allowed to this page");
			return -1;
		}else{
			memory[PFN*16 + offset] = (char)val;//store
			printf("Stored value %d at virtual address %d (physical address %d)\n", val, virt_addr, PFN*16 + offset);
			return 1;
		}
	}
	return 0;
}

int load(int PID, int virt_addr){
	int VPN = virt_addr/16;
	int offset = virt_addr - VPN*16;
	int PFN = 0;
	int value = 0;

	if(hardwareReg[PID] > 3){
		//this means page table is on disk, need to swap it in.
		openPFNTable = checkFreeList();
		if (openPFNTable == -1){
			swap(PID);
			openPFNTable = checkFreeList();
		}
		swapIn(PID, VPN, openPFNTable, 1); // swapping in a table.
		// Then, we still need to swap the page
		openPFNPage = checkFreeList();
		if (openPFNPage == -1){
			swap(PID); // swappin out a page
			openPFNPage = checkFreeList();
		}
		swapIn(PID, VPN, openPFNPage, 1); // swapping da page
		value = memory[openPFNPage*16 + offset];
		printf("The value %d is virtual address %d (physical address %d)\n", value, virt_addr, openPFNPage*16 + offset);
		return 0;
	}
	else if(memory[(hardwareReg[PID]*16) + VPN*4 + 2] > 3){
		// we just need to swap in a page
		openPFNPage = checkFreeList();
		if (openPFNPage == -1){
			swap(PID); // swappin out a page
			openPFNPage = checkFreeList();
		}
		swapIn(PID, VPN, openPFNPage, 0); // swapping da page
		value = memory[openPFNPage*16 + offset];
		printf("The value %d is virtual address %d (physical address %d)\n", value, virt_addr, openPFNPage*16 + offset);
		return 0;
	}else{
		PFN = memory[(hardwareReg[PID]*16) + VPN*4 + 2];
		value = memory[PFN*16 + offset];
		printf("The value %d is virtual address %d (physical address %d)\n", value, virt_addr, PFN*16 + offset);
		return 0;
	}
	return 1;
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
						if(atoi(value) < 0 || atoi(value) > 255){
							puts("When using store, values must be between 0 and 255.");
						}else{
							PID = atoi(pid);
							address = atoi(virt_addr);
							val = atoi(value);
							store(PID, address, val);
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

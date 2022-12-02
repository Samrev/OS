#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <stdbool.h>

/*Address space is of 32 bits and each page size if of 4KB, so 
maximum number of pages are (1<<20)*/

#define MAX (1<<20)
#define N  10000010

#define page_table_entry unsigned int

page_table_entry pages[MAX];

/* 
valid bit - 0
dirty bit - 1
frame - last 16 bits
*/
int frames[1000];
int capacity;
int verbose;
long int input[N];
int next[N];
char modes[N];
struct STATS{
	int writes;
	int misses;
	int drops ;
	int access;
} stats;
void printStats(){

	printf("Number of memory accesses: %d\n", stats.access);
	printf("Number of misses: %d\n", stats.misses);
	printf("Number of writes: %d\n", stats.writes);
	printf("Number of drops: %d\n", stats.drops);
    return ;
}
int isvalid(page_table_entry pte){
	return (pte & 1);
}
int isdirty(page_table_entry pte){
	return ((pte & 2)>>1);
} 
int type(char mode){
	return (mode == 'R')? 0 : 1;
}
int getframeno(page_table_entry pte){
	return (pte>>16);
}
void replace(int evict, long int VPN, char mode){
	long int oldVPN = frames[evict];
	if(isdirty(pages[oldVPN])){
		stats.writes++;
		if(verbose == 1){
			printf("Page 0x%05lx was read from disk, page 0x%05lx was written to the disk.\n", VPN ,oldVPN);		
		}
	}
	else{
		stats.drops++;
		if(verbose == 1){
			printf("Page 0x%05lx was read from disk, page 0x%05lx was dropped (it was not dirty).\n", VPN, oldVPN);		
		}
	}
	pages[oldVPN] =0;
	pages[VPN] = (evict << 16) | 1;
	frames[evict] = VPN;
	if(type(mode) == 1){
		pages[VPN] |= 2;	
	}
}
void allotnext(int VPN,int alloted, char mode){
	page_table_entry pte= (alloted<<16) | 1;
	if(type(mode) == 1){
		pte |= 2;	
	}
	pages[VPN] = pte;
	frames[alloted] = VPN;
}
void FIFO(char* trace){
	FILE* file = fopen(trace, "r");

	if(!file){
		printf("\n Unable to read the trace file: %s ", trace);
		return;
	} 

	char line[100];
	char* sep = " ";
	char address[60], mode;
	int alloted = 0, next = 0;
	while(fgets(line, sizeof(line),file)){
		stats.access++;
		sscanf(line, "%s %c",address,&mode);
		long int VPN = ((strtol(address, NULL, 0))>>12);
		/*check if the page is present in physical memory*/

		if(isvalid(pages[VPN])){
			if(type(mode) == 1){
				pages[VPN] |= 2;
			}
			continue;
		}
		stats.misses++;
		if(alloted < capacity){				
			allotnext(VPN,alloted++,mode);
		}
		else{
			replace(next,VPN,mode);
			next = (next + 1) % capacity;
		}
	}
}


void RAND(char* trace){
	FILE* file = fopen(trace, "r");

	if(!file){
		printf("\n Unable to read the trace file: %s ", trace);
		return;
	} 

	char line[100];
	char* sep = " ";
	char address[60], mode;
	int alloted = 0;
	while(fgets(line, sizeof(line),file)){
		stats.access++;
		sscanf(line, "%s %c",address,&mode);
		long int VPN = ((strtol(address, NULL, 0))>>12);
		/*check if the page is present in physical memory*/

		if(isvalid(pages[VPN])){
			if(type(mode) == 1){
				pages[VPN] |= 2;
			}
			continue;
		}
		stats.misses++;
		if(alloted < capacity){
			allotnext(VPN,alloted++,mode);
		}
		else{
			replace(rand() % capacity, VPN,mode);
		}
	}
}

void LRU(char* trace){
	FILE* file = fopen(trace, "r");

	if(!file){
		printf("\n Unable to read the trace file: %s ", trace);
		return;
	} 

	char line[100];
	char* sep = " ";
	char address[60], mode;
	int alloted = 0;
	int timeframe[capacity];
	while(fgets(line, sizeof(line),file)){
		stats.access++;
		sscanf(line, "%s %c",address,&mode);
		long int VPN = ((strtol(address, NULL, 0))>>12);
		/*check if the page is present in physical memory*/

		if(isvalid(pages[VPN])){
			timeframe[getframeno(pages[VPN])] = stats.access;
			if(type(mode) == 1){
				pages[VPN] |= 2;
			}
			continue;
		}
		stats.misses++;
		if(alloted < capacity){
			timeframe[alloted] = stats.access;
			allotnext(VPN,alloted++,mode);
		}
		else{
			int frameno = 0, leasttime = timeframe[0];
			for(int i = 1 ; i<capacity; i++){
				if(timeframe[i] < leasttime){
					leasttime = timeframe[i];
					frameno = i;
				}
			}
			timeframe[frameno] = stats.access;
			replace(frameno, VPN,mode);
		}
	}
}

void CLOCK(char* trace){
	FILE* file = fopen(trace, "r");

	if(!file){
		printf("\n Unable to read the trace file: %s ", trace);
		return;
	} 

	char line[100];
	char* sep = " ";
	char address[60], mode;
	int alloted = 0;
	int used[capacity];
	int curr_page = 0;
	while(fgets(line, sizeof(line),file)){
		stats.access++;
		sscanf(line, "%s %c",address,&mode);
		long int VPN = ((strtol(address, NULL, 0))>>12);
		/*check if the page is present in physical memory*/
		if(isvalid(pages[VPN])){
			used[getframeno(pages[VPN])] = 1;
			if(type(mode) == 1){
				pages[VPN] |= 2;
			}
			continue;
		}
		stats.misses++;
		if(alloted < capacity){
			used[alloted] = 1;
			allotnext(VPN,alloted++,mode);
		}
		else{
			while(used[curr_page] == 1){
				used[curr_page] = 0;
				curr_page = (curr_page + 1)%capacity;
			}
			replace(curr_page, VPN,mode);
			used[curr_page] = 1;
			curr_page = (curr_page+1)%capacity;
		}

	}
}

void OPT(char* trace){
	FILE* file = fopen(trace, "r");

	if(!file){
		printf("\n Unable to read the trace file: %s ", trace);
		return;
	} 

	char line[100];
	char* sep = " ";
	char address[60], mode;
	int alloted = 0;
	int i = 0;
	int prev_index[MAX];
	for(int i = 0 ; i<MAX; i++){
		prev_index[i] = -1;
	}
	while(fgets(line, sizeof(line),file)){
		stats.access++;
		sscanf(line, "%s %c",address,&mode);
		long int VPN = ((strtol(address, NULL, 0))>>12);
		// VPN = sim[i];
		input[i] = VPN, modes[i] = mode;
		if(prev_index[VPN] != -1){
			next[prev_index[VPN]] = i;
		}
		prev_index[VPN] = i;
		i++;
	}
	for(int j = 0 ; j<MAX; j++){
		prev_index[j] = -1;
	}
	for(int j = 0 ; j<i ; j++){
		long int VPN = input[j];
		mode = modes[j];
		prev_index[VPN] = j;
		if(isvalid(pages[VPN])){
			if(type(mode) == 1){
				pages[VPN] |= 2;
			}
		}
		else{
			stats.misses++;
			if(alloted<capacity){
				allotnext(VPN,alloted++,mode);
			}
			else{
				int frameno = 0, farthest = -1;
				for(int k = 0;k<capacity;k++){
					int tempVPN =  frames[k];
					if(next[prev_index[tempVPN]] == -1){
						frameno = k;
						break;
					}
					if(farthest < next[prev_index[tempVPN]]){
						farthest = next[prev_index[tempVPN]];
						frameno = k;
					}


				}
				replace(frameno,VPN,mode);

			}
		}
		
	}


}

int main(int argc , char** argv){
	struct STATS stats = {0,0,0,0};

	for(int i = 0 ; i<MAX; i++){
		pages[i] = 0;
	}
	for(int i = 0 ; i<N;i++){
		next[i] = -1;
	}
	/*setting the seed for RAND strategy*/

	srand(5635);
	capacity = strtol(argv[2],NULL,0);
	verbose = 0;
	if(argc>=5){
		if(strcmp(argv[4], "-verbose") == 0){
			verbose = 1;
		}
		else{
			printf("Invalid flag\n");
			return 0;
		}
	}
	if(strcmp(argv[3],"FIFO\0") == 0){
		FIFO(argv[1]);
	}
	else if(strcmp(argv[3],"RANDOM\0") == 0){
		RAND(argv[1]);
	}
	else if(strcmp(argv[3],"LRU\0") == 0){
		LRU(argv[1]);
	}
	else if(strcmp(argv[3],"CLOCK\0") == 0){
		CLOCK(argv[1]);
	}
	else if(strcmp(argv[3],"OPT\0") == 0){
		OPT(argv[1]);
	}
	printStats();
	return 0;
}


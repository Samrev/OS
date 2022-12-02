#include "mmu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define PAGE_TABLE_SIZE  (4096)
enum { CODE_MASK = 13, RO_MASK = 9, RW_MASK = 11, STACK_MASK = 11,NOT_ALLOTED = 16,
    PFN_MASK = (((1<<16)-1)<<16), BIT_MASK = ((1<<5) - 1)};

// byte addressable memory
unsigned char RAM[RAM_SIZE];  


// OS's memory starts at the beginning of RAM.
// Store the process related info, page tables or other data structures here.
// do not use more than (OS_MEM_SIZE: 72 MB).
unsigned char* OS_MEM = RAM;  

// memory that can be used by processes.   
// 128 MB size (RAM_SIZE - OS_MEM_SIZE)
unsigned char* PS_MEM = RAM + OS_MEM_SIZE; 


// This first frame has frame number 0 and is located at start of RAM(NOT PS_MEM).
// We also include the OS_MEM even though it is not paged. This is 
// because the RAM can only be accessed through physical RAM addresses.  
// The OS should ensure that it does not map any of the frames, that correspond
// to its memory, to any process's page. 
int NUM_FRAMES = ((RAM_SIZE) / PAGE_SIZE);

// Actual number of usable frames by the processes.
int NUM_USABLE_FRAMES = ((RAM_SIZE - OS_MEM_SIZE) / PAGE_SIZE);
#define PAGE_TABLE (NUM_USABLE_FRAMES)
#define PROCESS_TABLE (NUM_USABLE_FRAMES + 200*PAGE_TABLE_SIZE)

// To be set in case of errors. 
int error_no; 



void os_init() {
    // TODO student 
    // initialize your data structures.

   /* OS Memory layout: 
     *  0     ----------------------
     *           FRAMES(to check which frame is free)
     *  32767 ----------------------  
     *           PROCESSES TABLE(EACH PAGE TABLE size is 4096)
     *         --------------------- 
     *           PROCESSES TABLE
     *         ----------------------
    */
    memset(OS_MEM, 0 , NUM_USABLE_FRAMES*sizeof(unsigned char));
    struct PCB PROCESS[200];
    for(int i = 0 ; i<200; i++){
        PROCESS[i].pid = -1;
        PROCESS[i].page_table = (page_table_entry *)(OS_MEM + NUM_USABLE_FRAMES + i*PAGE_TABLE_SIZE);
        PROCESS[i].num_pages = 0;
    }
    memcpy(OS_MEM + PROCESS_TABLE, PROCESS , sizeof(PROCESS));
    struct PCB* PR = (struct PCB*)(OS_MEM + PROCESS_TABLE);
}


// ----------------------------------- Functions for managing memory --------------------------------- //

/**
 *  Process Virtual Memory layout: 
 *  ---------------------- (virt. memory start 0x00)
 *        code
 *  ----------------------  
 *     read only data 
 *  ----------------------
 *     read / write data
 *  ----------------------
 *        heap
 *  ----------------------
 *        stack  
 *  ----------------------  (virt. memory end 0x3fffff)
 * 
 * 
 *  code            : read + execute only
 *  ro_data         : read only
 *  rw_data         : read + write only
 *  stack           : read + write only
 *  heap            : (protection bits can be different for each heap page)
 * 
 *  assume:
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all in bytes
 *  code_size, ro_data_size, rw_data_size, max_stack_size, are all multiples of PAGE_SIZE
 *  code_size + ro_data_size + rw_data_size + max_stack_size < PS_VIRTUAL_MEM_SIZE
 *  
 * 
 *  The rest of memory will be used dynamically for the heap.
 * 
 *  This function should create a new process, 
 *  allocate code_size + ro_data_size + rw_data_size + max_stack_size amount of physical memory in PS_MEM,
 *  and create the page table for this process. Then it should copy the code and read only data from the
 *  given `unsigned char* code_and_ro_data` into processes' memory.
 *   
 *  It should return the pid of the new process.  
 *  
 */


int create_ps(int code_size, int ro_data_size, int rw_data_size,
                 int max_stack_size, unsigned char* code_and_ro_data) 
{   
    /* 0th bit ->read
       1st bit -> write
       2nd bit -> execute
       3rd bit -> present 
    */  
    int no_of_pages = (code_size +  ro_data_size + rw_data_size + max_stack_size)/PAGE_SIZE;
    int no_of_stack_pages = max_stack_size/PAGE_SIZE;
    int no_of_code_pages = code_size/PAGE_SIZE;
    int no_of_ro_pages = ro_data_size/PAGE_SIZE;
    int no_of_rw_pages = rw_data_size/PAGE_SIZE;

    
    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);

    /*check which pid is free*/
    for(int id = 0 ; id<200; id++){
        if(PROCESS[id].pid != -1){
            continue;
        }
        PROCESS[id].pid = id;
        page_table_entry *pg = PROCESS[id].page_table;
        PROCESS[id].num_pages = no_of_pages;
        int curr_page = 0;
        for(int frame=0 ; frame<NUM_USABLE_FRAMES ; frame++){
            if(curr_page >= 1024){
                break;
            }
            if(OS_MEM[frame] == 1){
                continue;
            }
            OS_MEM[frame] = 1;
            if(curr_page>=(1024 - no_of_stack_pages)){
                 pg[curr_page] = (frame<<16) | (STACK_MASK);
            }
            else if(curr_page>=(no_of_pages - no_of_stack_pages)){
                pg[curr_page] = NOT_ALLOTED;
                OS_MEM[frame] = 0;
                frame--;
            }
            else if(curr_page>=(no_of_code_pages + no_of_ro_pages)){
                pg[curr_page] = (frame<<16) | (RW_MASK);
            }
            else if(curr_page>=no_of_code_pages){
                pg[curr_page] = (frame<<16) | (RO_MASK);
                memcpy(PS_MEM + frame*PAGE_SIZE,code_and_ro_data+curr_page*PAGE_SIZE  , PAGE_SIZE);
            }
            else{
                pg[curr_page] = (frame<<16) | (CODE_MASK);
                memcpy(PS_MEM + frame*PAGE_SIZE,code_and_ro_data+curr_page*PAGE_SIZE , PAGE_SIZE);
            }

            curr_page++;
        }
        return id;
    }
    // 
    
}

/**
 * This function should deallocate all the resources for this process. 
 * 
 */
void exit_ps(int pid) 
{
   // TODO student
    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);
    PROCESS[pid].pid = -1;
    page_table_entry *page_table = PROCESS[pid].page_table;
    for(int i = 0 ; i<1024; i++){
        int frame = pte_to_frame_num(page_table[i]);
        int mask = (page_table[i] & BIT_MASK);
        if(mask == NOT_ALLOTED){
            continue;
        }
        page_table[i] = NOT_ALLOTED;
        OS_MEM[frame] = 0;
    }
    PROCESS[pid].num_pages=0;
}



/**
 * Create a new process that is identical to the process with given pid. 
 * 
 */
int fork_ps(int pid) {

    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);
    page_table_entry *page_table = PROCESS[pid].page_table;
    for(int id=0 ; id<200; id++){
        if(PROCESS[id].pid != -1){
            continue;
        }
        PROCESS[id].pid = id;
        page_table_entry *fork_page_table = PROCESS[id].page_table;
        PROCESS[id].num_pages = PROCESS[pid].num_pages;
        int free_frame =0;
        for(int i = 0 ; i<1024 ; i++){
            int frame = pte_to_frame_num(page_table[i]);
            int mask = (page_table[i] & BIT_MASK);
            if(mask == NOT_ALLOTED){
                continue;
            }
            for(free_frame ; free_frame<NUM_USABLE_FRAMES ; free_frame++){
                if(OS_MEM[free_frame] == 0){
                    break;
                }
            }
            OS_MEM[free_frame] = 1;
            fork_page_table[i] = (free_frame<<16) | mask;
            memcpy(PS_MEM + free_frame*PAGE_SIZE , PS_MEM + frame*PAGE_SIZE, PAGE_SIZE);
        }
        return id;

    }
    return 0;
}



// dynamic heap allocation
//
// Allocate num_pages amount of pages for process pid, starting at vmem_addr.
// Assume vmem_addr points to a page boundary.  
// Assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
//
//
// Use flags to set the protection bits of the pages.
// Ex: flags = O_READ | O_WRITE => page should be read & writeable.
//
// If any of the pages was already allocated then kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
void allocate_pages(int pid, int vmem_addr, int num_pages, int flags) 
{
    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);
    page_table_entry *page_table = PROCESS[pid].page_table;
    int starting_page = vmem_addr/PAGE_SIZE;
    if((starting_page + num_pages)>=1024){
        exit_ps(pid);
        error_no = ERR_SEG_FAULT;
        return;
    }
    int free_frame =0;
    for(int i = starting_page ; i<(starting_page + num_pages); i++){
        int frame = pte_to_frame_num(page_table[i]); 
        int mask = (page_table[i] & BIT_MASK);
        if(mask != NOT_ALLOTED){
            exit_ps(pid);
            error_no = ERR_SEG_FAULT;
            return;
        }
        for(free_frame ; free_frame<NUM_USABLE_FRAMES ; free_frame++){
            if(OS_MEM[free_frame] == 0){
                break;
            }
        }
        OS_MEM[free_frame] = 1;
        page_table[i] = (free_frame<<16) | flags;
    }
    PROCESS[pid].num_pages+=num_pages;
}



// dynamic heap deallocation
//
// Deallocate num_pages amount of pages for process pid, starting at vmem_addr.
// Assume vmem_addr points to a page boundary
// Assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE

// If any of the pages was not already allocated then kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
void deallocate_pages(int pid, int vmem_addr, int num_pages) 
{
    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);
    page_table_entry *page_table = PROCESS[pid].page_table;
    int starting_page = vmem_addr/PAGE_SIZE;
    if((starting_page + num_pages)>=1024){
        exit_ps(pid);
        error_no = ERR_SEG_FAULT;
        return;
    }
    for(int i = starting_page ; i<(starting_page + num_pages); i++){
        int frame = pte_to_frame_num(page_table[i]);
        int mask = (page_table[i] & BIT_MASK);
        if((mask == NOT_ALLOTED)){
            exit_ps(pid);
            error_no = ERR_SEG_FAULT;
            return;
        }
        page_table[i] = NOT_ALLOTED;
        OS_MEM[frame] = 0;
    }
    PROCESS[pid].num_pages-=num_pages;
}

// Read the byte at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
unsigned char read_mem(int pid, int vmem_addr) 
{
    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);
    page_table_entry *page_table = PROCESS[pid].page_table;
    int page_no = vmem_addr/PAGE_SIZE;
    int frame = pte_to_frame_num(page_table[page_no]);
    int mask = (page_table[page_no] & BIT_MASK);
    /*page boudary , if page is allocated and if the page read bit is set*/
    if(page_no>=1024 || (mask == NOT_ALLOTED) || (is_readable(page_table[page_no]) == 0)){
        exit_ps(pid);
        error_no = ERR_SEG_FAULT;
        return error_no;
    }
    unsigned char re = *(PS_MEM + PAGE_SIZE*((page_table[page_no] & PFN_MASK)>>16) + (vmem_addr % PAGE_SIZE));
    return re;
}

// Write the given `byte` at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
void write_mem(int pid, int vmem_addr, unsigned char byte) 
{
    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);
    page_table_entry *page_table = PROCESS[pid].page_table;
    int page_no = vmem_addr/PAGE_SIZE;
    int frame = pte_to_frame_num(page_table[page_no]);
    int mask = (page_table[page_no] & BIT_MASK);
    /*page boudary , if page is allocated and if the page write bit is set*/
    if(page_no>=1024 || (mask == NOT_ALLOTED) || (is_writeable(page_table[page_no]) == 0)){
        exit_ps(pid);
        error_no = ERR_SEG_FAULT;
        return;
    }
    *(PS_MEM + PAGE_SIZE*((page_table[page_no] & PFN_MASK)>>16) + (vmem_addr % PAGE_SIZE)) = byte;
}





// ---------------------- Helper functions for Page table entries ------------------ // 

// return the frame number from the pte
int pte_to_frame_num(page_table_entry pte) 
{
    return ((pte & PFN_MASK)>>16);
}


// return 1 if read bit is set in the pte
// 0 otherwise
int is_readable(page_table_entry pte) {
    return (pte & 1);
}

// return 1 if write bit is set in the pte
// 0 otherwise
int is_writeable(page_table_entry pte) {
    return ((pte & 2) > 0);
}

// return 1 if executable bit is set in the pte
// 0 otherwise
int is_executable(page_table_entry pte) {
    return ((pte & 4) > 0);
}


// return 1 if present bit is set in the pte
// 0 otherwise
int is_present(page_table_entry pte) {
    return ((pte & 8) > 0);
}

// -------------------  functions to print the state  --------------------------------------------- //

void print_page_table(int pid) 
{
    struct PCB* PROCESS = (struct PCB*)(OS_MEM + PROCESS_TABLE);
    page_table_entry* page_table_start = PROCESS[pid].page_table; // TODO student: start of page table of process pid
    int num_page_table_entries = 1024;           // TODO student: num of page table entries

    // Do not change anything below
    puts("------ Printing page table-------");
    for (int i = 0; i < num_page_table_entries; i++) 
    {
        page_table_entry pte = page_table_start[i];
        printf("Page num: %d, frame num: %d, R:%d, W:%d, X:%d, P%d\n", 
                i, 
                pte_to_frame_num(pte),
                is_readable(pte),
                is_writeable(pte),
                is_executable(pte),
                is_present(pte)
                );
    }

}

// add this to the mmu.c file and run

#include <assert.h>

#define MB (1024 * 1024)

#define KB (1024)

// just a random array to be passed to ps_create
unsigned char code_ro_data[10 * MB];


int main() {

    os_init();
    
    code_ro_data[10 * PAGE_SIZE] = 'c';   // write 'c' at first byte in ro_mem
    code_ro_data[10 * PAGE_SIZE + 1] = 'd'; // write 'd' at second byte in ro_mem

    int p1 = create_ps(10 * PAGE_SIZE, 1 * PAGE_SIZE, 2 * PAGE_SIZE, 1 * MB, code_ro_data);

    error_no = -1; // no error


    
    unsigned char c = read_mem(p1, 10 * PAGE_SIZE);

    assert(c == 'c');

    unsigned char d = read_mem(p1, 10 * PAGE_SIZE + 1);
    assert(d == 'd');

    assert(error_no == -1); // no error


    write_mem(p1, 10 * PAGE_SIZE, 'd');   // write at ro_data

    assert(error_no == ERR_SEG_FAULT);  


    int p2 = create_ps(1 * MB, 0, 0, 1 * MB, code_ro_data); // no ro_data, no rw_data

    error_no = -1; // no error


    int HEAP_BEGIN = 1 * MB;  // beginning of heap

    // allocate 250 pages
    allocate_pages(p2, HEAP_BEGIN, 250, O_READ | O_WRITE);

    write_mem(p2, HEAP_BEGIN + 1, 'c');

    write_mem(p2, HEAP_BEGIN + 2, 'd');

    assert(read_mem(p2, HEAP_BEGIN + 1) == 'c');

    assert(read_mem(p2, HEAP_BEGIN + 2) == 'd');

    deallocate_pages(p2, HEAP_BEGIN, 10);

    print_page_table(p2); // output should atleast indicate correct protection bits for the vmem of p2.

    write_mem(p2, HEAP_BEGIN + 1, 'd'); // we deallocated first 10 pages after heap_begin

    assert(error_no == ERR_SEG_FAULT);


    int ps_pids[100];

    // requesting 2 MB memory for 64 processes, should fill the complete 128 MB without complaining.   
    for (int i = 0; i < 64; i++) {
        ps_pids[i] = create_ps(1 * MB, 0, 0, 1 * MB, code_ro_data);
        print_page_table(ps_pids[i]);   // should print non overlapping mappings.  
    }


    exit_ps(ps_pids[0]);
    

    ps_pids[0] = create_ps(1 * MB, 0, 0, 500 * KB, code_ro_data);

    print_page_table(ps_pids[0]);   

    // allocate 500 KB more
    allocate_pages(ps_pids[0], 1 * MB, 125, O_READ | O_READ | O_EX);

    for (int i = 0; i < 64; i++) {
        print_page_table(ps_pids[i]);   // should print non overlapping mappings.  
    }
}


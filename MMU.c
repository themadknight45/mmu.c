// add this to the mmu.c file and run

#include <assert.h>
#define MB (1024 * 1024)
#define KB (1024)
#include "mmu.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// just a random array to be passed to ps_create
unsigned char code_ro_data[10 * MB];
unsigned int* processes;
unsigned int* pageframes;
static int cur_pf=0;
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

// To be set in case of errors. 
int error_no; 



void os_init() {

    unsigned char* temp= OS_MEM;
    for(int i=0;i<100;i++){
        struct PCB sa = { .pid = i+1, .page_table = temp + sizeof(sa) };  
        memcpy( temp, &sa, sizeof(sa));

        struct PCB* sap = (struct PCB*) (temp);
        temp=temp+sizeof(sa)+1024*4;
    }
    processes=temp;
    int* temp1=temp;
    for(int i=0;i<100;i++){
        *temp1=0;
        temp1++;
    }
    pageframes=temp1;
    for(int i=0;i<32768;i++){
        *temp1=0;
        temp1++;
    }
    
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

int find_phy_frame(){
    // while(1){
    //     
    //     if(*(pageframes+cur_pf)==0){
    //         *(pageframes+cur_pf)=1;
    //         return cur_pf;
    //     }
    //     else
    //     cur_pf=(cur_pf+1)%32768;
    // }
    for(int i=0;i<32768;i++){
        if(*(pageframes+i)==0){
            *(pageframes+i)=1;
            return i;
        }
    }
}
int create_ps(int code_size, int ro_data_size, int rw_data_size,
                 int max_stack_size, unsigned char* code_and_ro_data) 
{   
    int code_frame= ceil(code_size,4096); 
    int ro_frame=ceil(ro_data_size,4096);  
    int rw_frame=ceil(rw_data_size,4096);
    int stack_frame=ceil(max_stack_size,4096);
    unsigned int* x=processes;
    int pid=1;
    while(*x == 1){
        x++;pid++;
    }
    *x=1;
    unsigned int* pt_start =OS_MEM+4112*(pid-1)+16;
    unsigned int* st_start =OS_MEM+4112*pid-stack_frame*4;
    for(int i=0;i<code_frame;i++){
        int pte=0;
        int phy_frame=find_phy_frame();
        pte=phy_frame<<8;
        pte|=4;
        pte|=1;
        pte|=8;

            *pt_start=pte;
            pt_start++;
        if(i<code_frame-1){
            memcpy(PS_MEM+(phy_frame)*PAGE_SIZE,code_and_ro_data,PAGE_SIZE);
            code_size-=PAGE_SIZE;
            code_and_ro_data+=PAGE_SIZE;
        }
        else{
            memcpy(PS_MEM+phy_frame*PAGE_SIZE,code_and_ro_data,code_size);
            code_and_ro_data+=code_size;
            code_size=0;
        }
    }
    for(int i=0;i<ro_frame;i++){
        int pte=0;
        int phy_frame=find_phy_frame();
        pte=phy_frame<<8;
        pte|=1;
        pte|=8;
        *pt_start=pte;
        pt_start++;
        if(i<ro_frame-1){
            memcpy(PS_MEM+phy_frame*PAGE_SIZE,code_and_ro_data,PAGE_SIZE);
            ro_data_size-=PAGE_SIZE;
            code_and_ro_data+=PAGE_SIZE;
        }
        else{
            memcpy(PS_MEM+phy_frame*PAGE_SIZE,code_and_ro_data,ro_data_size);
            code_and_ro_data+=ro_data_size;
            ro_data_size=0;
        }
    }
    for(int i=0;i<rw_frame;i++){
        int pte=0;
        int phy_frame=find_phy_frame();
        pte=phy_frame<<8;
        pte|=1;pte|=2;pte|=8;
        *pt_start=pte;
        pt_start++;
    }
    for(int i=0;i<stack_frame;i++){
        int pte=0;
        int phy_frame=find_phy_frame();
        pte=phy_frame<<8;
        pte|=1;pte|=2;pte|=8;
        *st_start=pte;
        st_start++;
    }
    return pid;
    
}

int ceil(int x,int y){
    int ans=x/y;
    if(x%y!=0)ans++;
    return ans;
}
/**
 * This function should deallocate all the resources for this process. 
 * 
 */
void exit_ps(int pid) 
{
   *(processes+pid-1)=0;
    page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16; 
    for(int i=0;i<1024;i++){
        int pte=*(page_table_start+i);
        int phy_frame=(pte>>8);
        *(pageframes+phy_frame)=0;
        *(page_table_start+i)=0;
    }
}



/**
 * Create a new process that is identical to the process with given pid. 
 * 
 */
int fork_ps(int pid) {
    page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16;

    unsigned int* x=processes;
    int newpid=1;
    while(*x == 1){
        x++;newpid++;
    }
    *x=1;

    page_table_entry* new_ptb_start= OS_MEM+4112*(newpid-1)+16;

    for(int i=0;i<1024;i++){
        page_table_entry pte=*(page_table_start+i);
        int protection_bits=((255)& pte);
        *(new_ptb_start+i)=0;
        if(is_present(pte)){
            int phy_frame=find_phy_frame();
            page_table_entry new_pte=((phy_frame<<8)|protection_bits);
            *(new_ptb_start+i)=new_pte;
            memcpy(PS_MEM+phy_frame*PAGE_SIZE,PS_MEM+(pte>>8)*PAGE_SIZE,PAGE_SIZE);
        }
        
    }
    return newpid;
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
   page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16;
   int page_no=vmem_addr/PAGE_SIZE;
   page_table_start+=page_no;
   for(int i=0;i<num_pages;i++){
    int pte=0;
    int phy_frame=find_phy_frame();
    pte=phy_frame<<8;
    pte|=flags;pte|=8;
    if(is_present(*(page_table_start+i))==1){
        error_no=ERR_SEG_FAULT;
        exit_ps(pid);
        return;
    }
    else{
        *(page_table_start+i)=pte;
    }
   }
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
   page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16;
   int page_no=vmem_addr/PAGE_SIZE;
   page_table_start+=page_no;
   for(int i=0;i<num_pages;i++){
        int pte= *(page_table_start+i);
        if(!is_present(pte)){
            error_no=ERR_SEG_FAULT;
            exit_ps(pid);return;
        }
        int frame_no=pte>>8;
        *(pageframes+frame_no)=0;
        *(page_table_start+i)=0;
   }
}

// Read the byte at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE
unsigned char read_mem(int pid, int vmem_addr) 
{
        page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16;
        int page_no= vmem_addr/4096;
        int offset=vmem_addr%4096;
        page_table_start+=page_no;
        page_table_entry pte= *page_table_start;
        if(is_present(pte)==1){
            int phy_frame= pte>>8;

            char*read=PS_MEM+PAGE_SIZE*phy_frame+offset;
            return *read;
        }
        else{
            error_no=ERR_SEG_FAULT;
            exit_ps(pid);
            return NULL;
        }
}

// Write the given `byte` at `vmem_addr` virtual address of the process
// In case of illegal memory access kill the process, deallocate all its resources(ps_exit) 
// and set error_no to ERR_SEG_FAULT.
// 
// assume 0 <= vmem_addr < PS_VIRTUAL_MEM_SIZE

void write_mem(int pid, int vmem_addr, unsigned char byte) 
{
    page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16; 
    int page_no=vmem_addr/PAGE_SIZE;
    int offset=vmem_addr%PAGE_SIZE;
    page_table_entry pte=*(page_table_start+page_no);
    if(is_present(pte)!=1||is_writeable(pte)!=1){
            error_no=ERR_SEG_FAULT;
            exit_ps(pid);
            return;
    }
    else{
        int phy_frame=pte>>8;
        *(PS_MEM+phy_frame*PAGE_SIZE+offset)=byte;
    }
}





// ---------------------- Helper functions for Page table entries ------------------ // 

// return the frame number from the pte
int pte_to_frame_num(page_table_entry pte) 
{
    return pte>>8;
}


// return 1 if read bit is set in the pte
// 0 otherwise
int is_readable(page_table_entry pte) {
    if ((pte & (1)) ==0) {return 0;}
    return 1;
}

// return 1 if write bit is set in the pte
// 0 otherwise
int is_writeable(page_table_entry pte) {
    if((pte & 2) ==0) {return 0;}
    return 1;
}

// return 1 if executable bit is set in the pte
// 0 otherwise
int is_executable(page_table_entry pte) {
    if((pte & 4)==0) {return 0;}
    return 1;
}


// return 1 if present bit is set in the pte
// 0 otherwise
int is_present(page_table_entry pte) {
    if((pte & 8) ==0) {return 0;}
    return 1;
}

// -------------------  functions to print the state  --------------------------------------------- //

void print_page_table(int pid) 
{

    page_table_entry* page_table_start = OS_MEM+4112*(pid-1)+16; // TODO student: start of page table of process pid
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

int main() {

	os_init();
    printf("free pages before p1 = %d\n",free_pages());
	code_ro_data[10 * PAGE_SIZE] = 'c';   // write 'c' at first byte in ro_mem
	code_ro_data[10 * PAGE_SIZE + 1] = 'd'; // write 'd' at second byte in ro_mem

	int p1 = create_ps(10 * PAGE_SIZE, 1 * PAGE_SIZE, 2 * PAGE_SIZE, 1 * MB, code_ro_data);

	error_no = -1; // no error
    // printf("free pages = %d\n",free_pages());
    // print_page_table(1);
    
	unsigned char c = read_mem(p1, 10 * PAGE_SIZE);
    if(c=='c'){
        printf("test1 passed\n");
    }
	assert(c == 'c');

	unsigned char d = read_mem(p1, 10 * PAGE_SIZE + 1);
    if(d=='d'){
        printf("test2 passed\n");
    }
	assert(d == 'd');
    
	assert(error_no == -1); // no error
    // printf("free pages before write mem = %d\n",free_pages());
    // printf("trying to write\n");
	write_mem(p1, 10 * PAGE_SIZE, 'd');   // write at ro_data
    if(error_no==ERR_SEG_FAULT){
        printf("test3 passed\n");
    }
    
	assert(error_no == ERR_SEG_FAULT);  
    // return 0;
    // printf("free pages after write mem = %d\n",free_pages());
    // return 0;
	int p2 = create_ps(1 * MB, 0, 0, 1 * MB, code_ro_data);	// no ro_data, no rw_data
    
	error_no = -1; // no error

	int HEAP_BEGIN = 1 * MB;  // beginning of heap

	// allocate 250 pages
	allocate_pages(p2, HEAP_BEGIN, 250, O_READ | O_WRITE);
    // print_page_table(p2);return;
	write_mem(p2, HEAP_BEGIN + 1, 'c');
	write_mem(p2, HEAP_BEGIN + 2, 'd');
    
	
    if(read_mem(p2,HEAP_BEGIN+1)=='c'){
        printf("test4 passed\n");
    }
    else{
        printf("test4 failed\n");return 0;
    }
    assert(read_mem(p2, HEAP_BEGIN + 1) == 'c');

	assert(read_mem(p2, HEAP_BEGIN + 2) == 'd');
    if(read_mem(p2,HEAP_BEGIN+2)=='d'){
        printf("test5 passed\n");
    }
    

	deallocate_pages(p2, HEAP_BEGIN, 10);

	// print_page_table(p2); // output should atleast indicate correct protection bits for the vmem of p2.

    // return 0;

    
	write_mem(p2, HEAP_BEGIN + 1, 'd'); // we deallocated first 10 pages after heap_begin
    if(error_no==ERR_SEG_FAULT){
        printf("test6 passed\n");
    }
	assert(error_no == ERR_SEG_FAULT);
    // return 0;

    // int pid1=create_ps(1 * MB, 0, 0, 1 * MB, code_ro_data);
    // print_page_table(pid1);
    // return 0;

	int ps_pids[100];
    // exit_ps(0);
    // printf("free pages = %d\n",free_pages());
    // exit_ps(1);exit_ps(2);
    // printf("free pages = %d\n",free_pages());
    // return 0;


	// // requesting 2 MB memory for 64 processes, should fill the complete 128 MB without complaining.   
	for (int i = 0; i < 64; i++) {
    	ps_pids[i] = create_ps(1 * MB, 0, 0, 1 * MB, code_ro_data);
    	print_page_table(ps_pids[i]);	// should print non overlapping mappings.  
	}

    // return 0;

	exit_ps(ps_pids[0]);
	ps_pids[0] = create_ps(1 * MB, 0, 0, 500 * KB, code_ro_data);

	print_page_table(ps_pids[0]);   
    // return 0;
	// // allocate 500 KB more
	allocate_pages(ps_pids[0], 1 * MB, 125, O_READ | O_READ | O_EX);

	for (int i = 0; i < 64; i++) {
    	print_page_table(ps_pids[i]);	// should print non overlapping mappings.  
	}
    return 0;
}


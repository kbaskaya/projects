/**
 * virtmem.c 
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define TLB_SIZE 16
#define PAGES 1024
#define PAGE_MASK 1047552/* TODO *///which is 11111111110000000000 in binary

#define PAGE_SIZE 1024
#define OFFSET_BITS 10
#define OFFSET_MASK 1023/* TODO *///which is 0...01111111111 in binary

#define MEMORY_SIZE PAGES * PAGE_SIZE

// Max number of characters per line of input file to read.
#define BUFFER_SIZE 10

struct tlbentry {
  unsigned char logical;
  unsigned char physical;
};

// TLB is kept track of as a circular array, with the oldest element being overwritten once the TLB is full.
struct tlbentry tlb[TLB_SIZE];
// number of inserts into TLB that have been completed. Use as tlbindex % TLB_SIZE for the index of the next TLB line to use.
int tlbindex = 0;

// pagetable[logical_page] is the physical page number for logical page. Value is -1 if that logical page isn't yet in the table.
int pagetable[PAGES];
int validtable[PAGES];

signed char main_memory[256*PAGE_SIZE];//changed to decrease physical memory
int maintologic[256];

// Pointer to memory mapped backing file
signed char *backing;

int max(int a, int b)
{
  if (a > b)
    return a;
  return b;
}

/* Returns the physical address from TLB or -1 if not present. */
int search_tlb(unsigned char logical_page) {
    /* TODO */
    int i;
    for (i=0;i<TLB_SIZE;i++){
    	if(tlb[i].logical==logical_page)return tlb[i].physical;
    }
    return -1;
}

/* Adds the specified mapping to the TLB, replacing the oldest mapping (FIFO replacement). */
void add_to_tlb(unsigned char logical, unsigned char physical) {
    /* TODO */
    struct tlbentry new={logical,physical};
    tlb[tlbindex%TLB_SIZE]=new;
    tlbindex++;
}

int main(int argc, const char *argv[]) //----------------------------------------------------------MAIN-------------------------------------------------------------
{
  if (argc != 4) {
    fprintf(stderr, "Usage ./virtmem backingstore input [policy]\n");
    exit(1);
  }
  
  int counter = 0;
  int counters[256];
  
  const char *backing_filename = argv[1]; 
  int backing_fd = open(backing_filename, O_RDONLY);
  backing = mmap(0, MEMORY_SIZE, PROT_READ, MAP_PRIVATE, backing_fd, 0); 
  
  const char *input_filename = argv[2];
  FILE *input_fp = fopen(input_filename, "r");
  
  // Fill page table entries with -1 for initially empty table.
  int i;
  for (i = 0; i < PAGES; i++) {
    pagetable[i] = -1;
    validtable[i]=0;
  }
  for (i = 0; i < 256; i++) {
    maintologic[i]=-1;
    counters[i]=-1;
  }
    
  
  // Character buffer for reading lines of input file.
  char buffer[BUFFER_SIZE];
  
  // Data we need to keep track of to compute stats at end.
  int total_addresses = 0;
  int tlb_hits = 0;
  int page_faults = 0;
  
  // Number of the next unallocated physical page in main memory
  unsigned char free_page = 0;
  while (fgets(buffer, BUFFER_SIZE, input_fp) != NULL) {
    //counter++;printf("-------------------COUNTER:%d\n",counter);
    total_addresses++;
    int logical_address = atoi(buffer);

    /* TODO 
    / Calculate the page offset and logical page number from logical_address */
    int offset = logical_address & OFFSET_MASK;
    int logical_page = (logical_address & PAGE_MASK)>>OFFSET_BITS;
    ///////
    //printf("lOGİCAL PAGE:%d\n",logical_page);
    int physical_page = search_tlb(logical_page);
    // TLB hit
    if (physical_page != -1) {
      tlb_hits++;
      // TLB miss
    } else {
      if(validtable[logical_page]==0)physical_page=-1;//not in memory
      else {//in memory
      	physical_page = pagetable[logical_page];
      	counters[physical_page]=counter;
      }
      
      // Page fault
      if (physical_page == -1) {
          /* TODO */
          if(atoi(argv[3])==0){
          	int ii;
          	for(ii=0;ii<PAGE_SIZE;ii++){
          		main_memory[free_page*PAGE_SIZE+ii]=backing[logical_page*PAGE_SIZE+ii];
          	}
          	if(maintologic[free_page]!=-1){
          		validtable[maintologic[free_page]]=0;
          	}
          	maintologic[free_page]=logical_page;
          	pagetable[logical_page]=free_page;
          	validtable[logical_page]=1;
          	physical_page=free_page;
          	free_page=(free_page+1)%256;
          	page_faults++;
          }
          else{
          	int ii;
          	int min= 0;
          	for(ii=0;ii<256;ii++){
          		if(counters[ii]<counters[min])min=ii;
          	}
          	free_page=min;
          	//printf("COUNTERS İS: %d\n",counters[free_page]);
          	//printf("SELECTED FREE PAGE --> MEM:%d LPAGE:%d\n",free_page,maintologic[free_page]);
          	for(ii=0;ii<PAGE_SIZE;ii++){
          		main_memory[free_page*PAGE_SIZE+ii]=backing[logical_page*PAGE_SIZE+ii];
          	}
          	if(maintologic[free_page]!=-1){
          		validtable[maintologic[free_page]]=0;
          	}
          	maintologic[free_page]=logical_page;
          	counters[free_page]=counter;
          	pagetable[logical_page]=free_page;
          	validtable[logical_page]=1;
          	physical_page=free_page;
          	page_faults++;
          }
      }
      add_to_tlb(logical_page, physical_page);
    }
    
    int physical_address = (physical_page << OFFSET_BITS) | offset;
    signed char value = main_memory[physical_page * PAGE_SIZE + offset];
    
    printf("Virtual address: %d Physical address: %d Value: %d LPAGE: %d\n", logical_address, physical_address, value,logical_page);
  }
  
  printf("Number of Translated Addresses = %d\n", total_addresses);
  printf("Page Faults = %d\n", page_faults);
  printf("Page Fault Rate = %.3f\n", page_faults / (1. * total_addresses));
  printf("TLB Hits = %d\n", tlb_hits);
  printf("TLB Hit Rate = %.3f\n", tlb_hits / (1. * total_addresses));
  
  return 0;
}

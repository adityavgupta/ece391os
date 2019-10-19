#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"

#define TABLE_ENTRIES 1024
#define RW_NOT_PRESENT 0x00000002
#define RW_PRESENT 0x3
#define VIDEO_MEM_ADDR 0xB8000 //taken from lib.c
#define PT_OFFSET 12 //how many bits to right shift linear address to get offset in the page table
#define PAGE_SIZE 4096

static uint32_t page_directory[TABLE_ENTRIES]  __attribute__((aligned (PAGE_SIZE)));
static uint32_t page_table[TABLE_ENTRIES]  __attribute__((aligned (PAGE_SIZE)));


void init_paging(void){
  init_page_directory();
  init_page_table();
  enable_paging();
}

void init_page_directory(void){
  int i;
  for(i=0;i<TABLE_ENTRIES;i++){
    page_directory[i]=RW_NOT_PRESENT;//mark all table entries to not present
  }
  page_directory[0]=((unsigned int)page_table)|RW_PRESENT; //only one page table in page directory
}

void init_page_table(void){
  int i;
  for(i=0;i<TABLE_ENTRIES;i++){
    page_table[i]=(i*PAGE_SIZE)|RW_NOT_PRESENT;//mark all table entries to not present
  }
  page_table[VIDEO_MEM_ADDR>>PT_OFFSET]|=RW_PRESENT;//mark video mem page as present, rw and supervisor mode
}

void enable_paging(void){
  uint32_t enable = 0x80000001;
  asm volatile ("movl %0,%%eax\n\
              movl %%eax,%%cr3\n\
              movl %%cr0,%%eax\n\
              orl %1,%%eax\n\
              movl %%eax,%%cr0"
          :
          : "r"(page_directory),"r"(enable)
          : "eax"
  );
}

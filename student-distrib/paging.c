#include "types.h"
#include "lib.h"

#define TABLE_ENTRIES 1024

static uint32_t page_directory[TABLE_ENTRIES]  __attribute__((aligned (4096)));
static uint32_t page_table[TABLE_ENTRIES]  __attribute__((aligned (4096)));

void init_page_directory(void){
  memset(page_directory,0x00000002,TABLE_ENTRIES*4);
  page_directory[0]=(page_table&0x000)|0x3;
}

void init_page_table(void){
  int i;
  for(i=0;i<TABLE_ENTRIES;i++){
    page_table[i]=((i*4096)&0x000)|0x2;
  }
  page_table[VIDEO>>12]|=0x3;
}

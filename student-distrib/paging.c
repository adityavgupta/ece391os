#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "paging.h"
#include "file_system.h"

/* Constants for paging */
#define TABLE_ENTRIES 1024
#define RW_NOT_PRESENT 0x02
#define RW_PRESENT 0x03
#define VIDEO_MEM_ADDR 0xB8000
#define KERNEL_ADDR 0x400000
#define PT_OFFSET 12
#define PAGE_SIZE 4096
#define FOUR_MB 4194304
#define USER_PROG 0x08000000
#define PROG_OFFSET 0x00048000

/* Page directory array */
static uint32_t page_directory[TABLE_ENTRIES]  __attribute__((aligned (PAGE_SIZE)));
/* Page table array */
static uint32_t page_table[TABLE_ENTRIES]  __attribute__((aligned (PAGE_SIZE)));

/*
 * set_page_dir_entry
 *    DESCRIPTION: Creates an entry in the page directory
 *    INPUTS: int32_t virtual - the virtual address
 *            int32_t physical - the physical address to map to
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, -1 for failure
 *    SIDE EFFECTS:
 */
int32_t set_page_dir_entry(int32_t virtual, int32_t physical){
  /* Check if page already exists */
  if(page_directoryp[virtual >> 22] & 0x1){
    /* Return failure */
    return -1;
  }

  /* Create entry */
  page_directory[virtual >> 22] = physical | 0x083;

  /* Return success */
  return 0;
}

/*
 * set_page_table_entry
 *    DESCRIPTION: Enables a page in the page table
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: 0
 *    SIDE EFFECTS: none
 */
int32_t set_page_table_entry(){
  /* Nothing for now */
  return 0;
}

/*
 * get_dir
 *    DESCRIPTION: Get an entry from page directory
 *    INPUTS: uint32_t i - Index into page directory
 *    OUTPUTS: none
 *    RETURN VALUE: An entry from the page directory
 *    SIDE EFFECTS: none
 */
uint32_t get_dir(unsigned int i){
  return page_directory[i];
}

/*
 * get_page
 *    DESCRIPTION: Get an entry from page table
 *    INPUTS: uint32_t i - Index into page table
 *    OUTPUTS: none
 *    RETURN VALUE: An entry from the page table
 *    SIDE EFFECTS: none
 */
uint32_t get_page(unsigned int i){
  return page_table[i];
}

/*
 * init_paging
 *    DESCRIPTION: Begins paging initialization
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: none
 */
void init_paging(void){
    /* Initialize page directory array */
    init_page_directory();

    /* Initialize first page table array */
    init_page_table();

    /* Turn on paging */
    enable_paging();
}

/*
 * init_page_directory
 *    DESCRIPTION: Initializes the page directory
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Sets kernel page and first page table, all others not present
 */
void init_page_directory(void){
    int i; /* Variable to loop over table entries */

    /* Go through each entry in the table */
    for(i = 0;i < TABLE_ENTRIES; i++){
      /* Set entry to not present */
      page_directory[i] = RW_NOT_PRESENT;
    }

    /* Set the first entry to the address of the page table, and mark it as present */
    page_directory[0] = ((unsigned int)page_table) | RW_PRESENT;

    /*
     * Set the second entry to the address of the kernel (4 MB). The page size bit must be
     * enabled. The entry is marked as present as well.
     */
    page_directory[KERNEL_ADDR >> 22]= (KERNEL_ADDR | 0x083);
}

/*
 * init_page_table
 *    DESCRIPTION: Initializes the first page table
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Sets video memory page, all others not present
 */
void init_page_table(void){
    int i; /* Variable to loop */

    /* Go through each entry in the page table */
    for(i=0;i<TABLE_ENTRIES;i++){
      /* Address of each page is every 4 kiB (PAGE_SIZE), and entry is not present */
      page_table[i] = (i * PAGE_SIZE)| RW_NOT_PRESENT;
    }

    /* Set video memory page to present, read/write, and supervisor mode */
    page_table[VIDEO_MEM_ADDR >> PT_OFFSET]|= RW_PRESENT;
}

/*
 * enable_paging
 *    DESCRIPTION: Initializes paging
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Sets cr3 to hold the page directory address, and enables paging in cr0 and cr4
 */
void enable_paging(void){
    /* The most significant bit turns on paging */
    uint32_t enable = 0x80000001;

    /*
     * Macro to store address of page directory in cr3. Also enables paging in cr0, and sets the
     * Page Size bit in cr4 to allow for 4MB pages.
     */
    asm volatile ("\n\
                movl %%cr4,%%eax\n\
                or $0x10,%%eax\n\
                mov %%eax, %%cr4\n\
                movl %0,%%eax\n\
                movl %%eax,%%cr3\n\
                movl %%cr0,%%eax\n\
                orl %1,%%eax\n\
                movl %%eax,%%cr0"
            :
            : "r"(page_directory),"r"(enable)
            : "eax"
    );
}

//unclear if process num needed
int8_t program_loader(int8_t process_num,uint8_t* filename){
  uint8_t buf[FOUR_MB]; //4MB buffer
  int32_t fd = file_open(name);
  uint32_t size;
  if((size = file_read(fd, buf, FOUR_MB)) == -1){
    file_close(fd);
    return -1;
  }
  file_close(fd);
  memcpy((void*)(USER_PROG+PROG_OFFSET),(const void*)buf,size);//copy program into physical memory
  return 0;
}
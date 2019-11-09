#ifndef _PAGING_H
#define _PAGING_H

int8_t program_loader(int8_t process_num,uint8_t* filename);

/* Create an entry in the page directory */
int32_t set_page_dir_entry();

/* Create an entry in the page table */
int32_t set_page_table_entry();

/* Get a page directory entry */
uint32_t get_dir(uint32_t i);

/* Get a page table entry */
uint32_t get_page(uint32_t i);

/* Initialize the page directory */
void init_page_directory(void);

/* Initialize the page table */
void init_page_table(void);

/* Create page directories and page tables */
void init_paging(void);

/* Activate paging */
void enable_paging(void);

#endif /* _PAGING_H */

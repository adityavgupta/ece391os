#ifndef _PAGING_H
#define _PAGING_H

/* Get a page directory entry */
uint32_t get_dir(unsigned int i);

/* Get a page table entry */
uint32_t get_page(unsigned int i);

/* Initialize the page directory */
void init_page_directory(void);

/* Initialize the page table */
void init_page_table(void);

/* Create page directories and page tables */
void init_paging(void);

/* Activate paging */
void enable_paging(void);

#endif /* _PAGING_H */

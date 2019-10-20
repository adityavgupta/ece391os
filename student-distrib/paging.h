#ifndef _PAGING_H
#define _PAGING_H

/* Initialize the page directory */
void init_page_directory(void);

/* Initialize the page table */
void init_page_table(void);

/* Create page directories and page tables */
void init_paging(void);

/* Activate paging */
void enable_paging(void);

#endif

#include "syscalls.h"
#include "x86_desc.h"
#include "lib.h"

#define VIRTUAL_ADDR 0x08000000
#define 8MB 0x800000
#define 4MB 0x400000
#define PAGE_SIZE 0x400000
#define USER_PROG 0x08000000
#define PROG_OFFSET 0x00048000

static int32_t process_num = 0;

/*
 * halt
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
int32_t halt(uint8_t status){
  goto done; // sketchy af
  return 0;
}

/*
 * execute
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
int32_t execute(const uint8_t* command){
	uint8_t filename[32]; /* Name of the file */
  int i = 0; /* Loop variable */

  /* Mask interrupts */
  cli();

  /* Get the file name */
  while(command[i] != '\0' && command[i] != ' ' && i < 32){
    filename[i] = command[i];
    i++;
  }

  filename[i] = '\0';
  dentry_t file_dentry;
  /* Check for a valid file name */
  if(read_dentry_by_name(filename, &file_dentry) == -1){
    /* Return failure */
    return -1;
  }

  /* Read the executable */
  uint8_t ELF_buf[4MB];
  uint32_t size;
  if((read_data(file_dentry.inode_num, 0, ELF_buf, 4MB) = size) == -1){
    /* Return failure */
    return -1;
  }
  uint8_t elf[] = "ELF";
  /* Check if file is an executable */
  if(!(ELF_buf[0] == 0x7F && !strncmp((int8_t*)(ELF_buf + 1), (int8_t*)elf, 3))){
    /* Return failure */
    return -1;
  }

  /* Set up user page */
  set_page_dir_entry(VIRTUAL_ADDR, 8MB + (process_num++)*PAGE_SIZE);

  /* Flush tlb */
  asm volatile ("      \n\
     movl %%eax, %%cr3 \n\
     movl %%cr3, %%eax"
     :
     :
     : "eax"
  );

  // need to add pcb stuff here

  /* Load program into physical memory */
  memcpy((void*)(USER_PROG + PROG_OFFSET), (const void*)ELF_buf, size);

  /* Set TSS values */
  tss.esp0 = 8MB - process_num*0x2000;
  tss.ss0 = KERNEL_DS;

  /* Get address of first instruction */
  uint32_t program_addr = ELF_buf[23] << 24 || ELF_buf[24] << 16 || ELF_buf[25] << 8 || ELF_buf[26];

  goto context_switch; // jump to the context switch

done:

  /* Return success */
  return 69;
}

/*
 * read
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
  return 0;
}

/*
 * write
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
int32_t write(int32_t fd, void* buf, int32_t nbytes){
  return 0;
}

/*
 * open
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
int32_t open(const uint8_t* filename){
  return 0;
}

/*
 * close
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
int32_t close(int32_t fd){
  return 0;
}

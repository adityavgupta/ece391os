#include "syscalls.h"
#include "x86_desc.h"
#include "lib.h"

#define VIRTUAL_ADDR 0x08000000
#define 8MB 0x800000
#define PAGE_SIZE 0x400000

static int32_t process_num = -1;

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
	uint8_t filename[32];
  int i = 0;

  while(command[i] != '\0' && command[i] != ' ' && i < 32){
    filename[i] = command[i];
    i++;
  }

  filename[i] = '\0';
  dentry_t file_dentry;
  // invalid file name
  if(read_dentry_by_name(filename, &file_dentry) == -1){
    /* Return failure */
    return -1;
  }

  /* Read first 28 bytes of the executable */
  uint8_t ELF_buf[28];
  read_data(file_dentry.inode_num, 0, ELF_buf, 4);
  uint8_t elf[] = "ELF";
  // not an executable
  if(!(ELF_buf[0] == 0x7F && !strncmp((int8_t*)(ELF_buf + 1), (int8_t*)elf, 3))){
    /* Return failure */
    return -1; // check for ELF magic number
  }

  set_page_dir_entry(VIRTUAL_ADDR, 8MB + (++process_num)*PAGE_SIZE);

  /* Flush tlb */
  asm volatile ("      \n\
     movl %%eax, %%cr3 \n\
     movl %%cr3, %%eax"
     :
     :
     : "eax"
  );

  // need to add pcb stuff here

  if(program_loader(process_num, filename) == -1){
    /* Return failure */
    return -1;
  }

  tss.esp0 = 8MB - (process_num + 1)*0x2000;
  // potentially add ss0 change

  int32_t program_addr = ELF_buf[23] << 24 || ELF_buf[24] << 16 || ELF_buf[25] << 8 || ELF_buf[26];

  cli();

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

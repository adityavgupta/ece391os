#include "syscalls.h"
#include "x86_desc.h"
#include "paging.h"
#include "lib.h"
#include "kb.h"

#define EIGHT_MB 0x800000
#define FOUR_MB 0x400000
#define USER_PROG 0x08000000
#define PROG_OFFSET 0x00048000

jump_table rtc_table = {rtc_write, rtc_read, rtc_open, rtc_close};
jump_table file_table = {file_write, file_read, file_open, file_close};
jump_table dir_table = {dir_write, dir_read, dir_open, dir_close};

jump_table stdint_table = {NULL, terminal_read, NULL, NULL};
jump_table stdoutt_table = {terminal_write, NULL, NULL, NULL};
jump_table  descript3 = {NULL, NULL, NULL, NULL};
jump_table  descript4 = {NULL, NULL, NULL, NULL};
jump_table  descript5 = {NULL, NULL, NULL, NULL};
jump_table  descript6 = {NULL, NULL, NULL, NULL};
jump_table  descript7 = {NULL, NULL, NULL, NULL};

file_desc stdin_descr = {&stdint_table, -1, -1, 1};
file_desc stdout_descr = {&stdoutt_table, -1, -1, 1};
file_desc descr3 = {&descript3, -1, -1, 0};
file_desc descr4 = {&descript4, -1, -1, 0};
file_desc descr5 = {&descript5, -1, -1, 0};
file_desc descr6 = {&descript6, -1, -1, 0};
file_desc descr7 = {&descript7, -1, -1, 0};

file_desc* file_desc_table[8] = {&stdin_descr, &stdout_descr, &descr3, &descr4, &descr5, &descr6, &descr7};

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
  uint8_t ELF_buf[30];
  uint32_t size;
  if((size = read_data(file_dentry.inode_num, 0, ELF_buf, 30)) == -1){
    /* Return failure */
    return -1;
  }
  /* Set up user page */
  set_page_dir_entry(USER_PROG, EIGHT_MB + (process_num++)*FOUR_MB);

  /* Flush tlb */
  asm volatile ("      \n\
     movl %%cr3, %%eax \n\
     movl %%eax, %%cr3"
     :
     :
     : "eax"
  );

  if((size = read_data(file_dentry.inode_num, 0, (uint8_t*)(USER_PROG + PROG_OFFSET), 40000)) == -1){
    /* Return failure */
    return -1;
  }

  uint8_t elf[] = "ELF";
  /* Check if file is an executable */
  if(!(ELF_buf[0] == 0x7F && !strncmp((int8_t*)(ELF_buf + 1), (int8_t*)elf, 3))){
    /* Return failure */
    return -1;
  }

  //memcpy((void *)(EIGHT_MB - process_num*0x2000), pcb, sizeof(pcb));

  /* Set TSS values */
  tss.esp0 = EIGHT_MB - process_num*0x2000;
  tss.ss0 = KERNEL_DS;

  /* Get address of first instruction */
  uint32_t program_addr = *(uint32_t*)(ELF_buf+24);

  asm volatile("\n\
    movl %0, %%ecx \n\
    jmp context_switch"
    :
    : "r"(program_addr)
    : "ecx"
  );

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
  cli();
		if(fd < 0 || fd > 7){
				sti();
				return 0;
		}

		file_desc* curr_file= file_desc_table[fd];

		if(curr_file == NULL){
				sti();
				return 0;
		}

		if(curr_file->jump_ptr->read != NULL){
				sti();
				return curr_file->jump_ptr->read(fd,buf,nbytes);
		}
		sti();
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
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
  cli();

	if(fd < 0 || fd > 7){
		sti();
		return -1;
	}

	file_desc* curr_file = file_desc_table[fd];

	if(curr_file == NULL){
		sti();
		return -1;
	}

	if(curr_file->jump_ptr->write != NULL){
		sti();
		return curr_file->jump_ptr->write(fd,buf,nbytes);
	}

	sti();

	return -1;
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
  if(strncmp((int8_t*)filename, (int8_t*)"stdin", strlen((int8_t*)filename))==0 ){
		terminal_open(filename);
		return 0;
	}

	if(strncmp((int8_t*)filename, (int8_t*)"stdout", strlen((int8_t*)filename)) == 0){
		terminal_open(filename);
		return 1;
	}


	dentry_t temp_dentry;
	int temp = read_dentry_by_name((uint8_t*)filename, &temp_dentry);

	if(temp == -1){
		return -1;
	}

	int inode=temp_dentry.inode_num;

	int i;
	for(i=2;i<8;i++){
		if(file_desc_table[i]->flags == 0){
			if(strncmp((int8_t*)filename, (int8_t*)"rtc", strlen((int8_t*)filename)) == 0){
				file_desc_table[i]->flags = 3;
				file_desc_table[i]->jump_ptr = &rtc_table;
				file_desc_table[i]->inode = inode;
				file_desc_table[i]->file_position = 0;
				rtc_open((uint8_t*) filename);
				return i;
			}
			else if(strncmp((int8_t*)filename, (int8_t*)".", strlen((int8_t*)filename)) == 0){
				file_desc_table[i]->flags = 3;
				file_desc_table[i]->jump_ptr = &dir_table;
				file_desc_table[i]->inode = inode;
				file_desc_table[i]->file_position = 0;
				dir_open((uint8_t*) filename);
				return i;
			}
			else{
				file_desc_table[i]->flags = 1;
				file_desc_table[i]->jump_ptr = &file_table;
				file_desc_table[i]->inode = inode;
				file_desc_table[i]->file_position = 0;
				file_open((uint8_t*) filename);
				return i;
			}
		}
	}

	return -1;
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
  if(fd > 7 || fd < 0){
		return -1;
	}
	if(file_desc_table[fd]->flags == 1 || file_desc_table[fd]->flags == 3){
		if(file_desc_table[fd]->jump_ptr->close! = NULL){
			file_desc_table[fd]->jump_ptr->close(fd);
			file_desc_table[fd]->flags = 0;
		}
		return 0;
	}
	return -1;
}

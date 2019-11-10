#include "syscalls.h"
#include "x86_desc.h"
#include "paging.h"
#include "lib.h"
#include "kb.h"
#include "pcb.h"


#define EIGHT_MB 0x800000
#define FOUR_MB 0x400000
#define USER_PROG 0x08000000
#define PROG_OFFSET 0x00048000
#define RUNNING 0
#define STOPPED 1

jump_table rtc_table = {rtc_write, rtc_read, rtc_open, rtc_close};
jump_table file_table = {file_write, file_read, file_open, file_close};
jump_table dir_table = {dir_write, dir_read, dir_open, dir_close};

jump_table stdin_table = {NULL, terminal_read, NULL, NULL};
jump_table stdout_table = {terminal_write, NULL, NULL, NULL};

//file_desc* file_desc_table[8] = {&stdin_descr, &stdout_descr, &descr3, &descr4, &descr5, &descr6, &descr7};

int32_t process_num = 0;

/*
 * halt
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
int32_t halt(uint8_t status){
  if(process_num==1){
    process_num=0;
    uint8_t sh[]="shell";
    execute((uint8_t*)sh);
  }
  process_num=0;
  set_page_dir_entry(USER_PROG, EIGHT_MB + (process_num++)*FOUR_MB);
  /* Flush tlb */
  asm volatile ("      \n\
     movl %%cr3, %%eax \n\
     movl %%eax, %%cr3"
     :
     :
     : "eax"
  );
  //close relevant fds
  second_pcb.process_state=STOPPED;
  tss.esp0 = second_pcb.parent_esp;
  asm volatile ("      \n\
     movzbl %%bl,%%eax    \n\
     movl %0,%%ebp    \n\
     leave \n\
     ret"
     :
     : "r"(second_pcb.parent_ebp)
     : "ebp","eax"
  );
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

  pcb_t pcb;
  pcb.pid = process_num;
  pcb.fdt[0].jump_ptr = &stdin_table;
  pcb.fdt[1].jump_ptr = &stdout_table;
  pcb.process_state = 0;
  memcpy((void *)(EIGHT_MB - process_num*0x2000), &pcb, sizeof(pcb));

  /* Set TSS values */
  tss.esp0 = EIGHT_MB;
  tss.ss0 = KERNEL_DS;

  /* Get address of first instruction */
  uint32_t program_addr = *(uint32_t*)(ELF_buf+24);
  if(process_num==2){
    asm volatile("\n\
      movl %%esp, %0 \n\
      movl %%ebp, %1"
      :"=r"(second_pcb.parent_esp),"=r"(second_pcb.parent_ebp)
    );
  }
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
    pcb_t* pcb_start = get_pcb_add();
		file_desc curr_file= pcb_start->fdt[fd];

		if(curr_file.inode == -1){
				sti();
				return 0;
		}

		if(curr_file.jump_ptr->read != NULL){
				sti();
				return curr_file.jump_ptr->read(fd,buf,nbytes);
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

  pcb_t* pcb_start = get_pcb_add();
	file_desc curr_file = pcb_start->fdt[fd];

	if(curr_file.inode == -1){
		sti();
		return -1;
	}

	if(curr_file.jump_ptr->write != NULL){
		sti();
		return curr_file.jump_ptr->write(fd,buf,nbytes);
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

  pcb_t* pcb_start = get_pcb_add();

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
		if(pcb_start->fdt[i].flags == 0){
			if(strncmp((int8_t*)filename, (int8_t*)"rtc", strlen((int8_t*)filename)) == 0){
				pcb_start->fdt[i].flags = 3;
				pcb_start->fdt[i].jump_ptr = &rtc_table;
				pcb_start->fdt[i].inode = inode;
				pcb_start->fdt[i].file_position = 0;
				rtc_open((uint8_t*) filename);
				return i;
			}
			else if(strncmp((int8_t*)filename, (int8_t*)".", strlen((int8_t*)filename)) == 0){
				pcb_start->fdt[i].flags = 3;
				pcb_start->fdt[i].jump_ptr = &dir_table;
				pcb_start->fdt[i].inode = inode;
				pcb_start->fdt[i].file_position = 0;
				dir_open((uint8_t*) filename);
				return i;
			}
			else{
				pcb_start->fdt[i].flags = 1;
				pcb_start->fdt[i].jump_ptr = &file_table;
				pcb_start->fdt[i].inode = inode;
				pcb_start->fdt[i].file_position = 0;
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
  pcb_t* pcb_start = get_pcb_add();

  if(fd > 7 || fd < 0){
		return -1;
	}
	if(pcb_start->fdt[fd].flags == 1 || pcb_start->fdt[fd].flags == 3){
		if(pcb_start->fdt[fd].jump_ptr->close != NULL){
			pcb_start->fdt[fd].jump_ptr->close(fd);
			pcb_start->fdt[fd].flags = 0;
		}
		return 0;
	}
	return -1;
}

/*
 * get_espval
 *    DESCRIPTION:
 *    INPUTS:
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS:
 */
pcb_t* get_pcb_add (void) {
  int32_t espv;
  pcb_t* pcb_add;
  asm volatile("\n\
    movl %%esp, %0"
    : "=r" (espv)
  );
  pcb_add = (pcb_t*)(espv&0xFFFFE000); // 8kB = 2^13, so mask everything below 13th bit
  return pcb_add;
}

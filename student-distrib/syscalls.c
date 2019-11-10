#include "syscalls.h"
#include "x86_desc.h"
#include "paging.h"
#include "lib.h"
#include "kb.h"


#define EIGHT_MB 0x800000
#define FOUR_MB 0x400000
#define USER_PROG 0x08000000
#define PROG_OFFSET 0x00048000
#define RUNNING 0
#define STOPPED 1

 //function pointers for rtc
jump_table rtc_table = {rtc_write, rtc_read, rtc_open, rtc_close};

//function pointers for file
jump_table file_table = {file_write, file_read, file_open, file_close};

//function pointers for directory
jump_table dir_table = {dir_write, dir_read, dir_open, dir_close};

//function pointers for stdin(only has terminal read)
jump_table stdin_table = {NULL, terminal_read, NULL, NULL};

//function pointers for stdout(only has terminal write)
jump_table stdout_table = {terminal_write, NULL, NULL, NULL};

//process number: 1st process has pid 1, 0 means no processes have been launched
int32_t process_num = 0;

/*
 * halt
 *    DESCRIPTION: halt system call, restores parent process paging and data and closes fds
 *    INPUTS: status: return value from halt
 *    OUTPUTS: none
 *    RETURN VALUE:
 *    SIDE EFFECTS: halt current process, return to parent process(shell)
 */
int32_t halt(uint8_t status){
  if(process_num==1){ //if shell tries to halt, just launch shell again
    process_num=0;
    uint8_t sh[]="shell";
    execute((uint8_t*)sh);
  }
  //else we are in a child process
  int i;
  for(i=0;i<8;i++){ //close all files in the pcb
    close(i);
  }
  process_num--; //decrement process number
  set_page_dir_entry(USER_PROG, EIGHT_MB + (process_num - 1)*FOUR_MB);//remap user program paging back to parent program
  /* Flush tlb */
  asm volatile ("      \n\
     movl %%cr3, %%eax \n\
     movl %%eax, %%cr3"
     :
     :
     : "eax"
  );
  pcb_t* cur_pcb = get_pcb_add(); //get the current process pcb
  cur_pcb->process_state=STOPPED; //set process state to stopped
  tss.esp0 = cur_pcb->parent_esp; //set TSS esp0 back to parent stack pointer

  /*
  inline assembly:
    1st line: zero extend bl and move value into eax so that the correct status is returned from halt
    2nd line: set ebp to parent process ebp
    3rd andd 4th line: jump back to parent's system call linkage
  */
  asm volatile ("      \n\
     movzbl %%bl,%%eax    \n\
     movl %0,%%ebp    \n\
     leave \n\
     ret"
     :
     : "r"(cur_pcb->parent_ebp)
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
  pcb.fdt[0].flags = 1;
  pcb.fdt[1].flags = 1;
  pcb.process_state = 0;
  for(i = 2; i < 8; i++){
    pcb.fdt[i].flags = -1;
  }
  if(process_num >= 2){
    pcb.parent_esp = EIGHT_MB - (process_num-2)*0x2000;
    asm volatile("\n\
    movl %%ebp, %0"
    : "=r"(pcb.parent_ebp)
    );
  }
  memcpy((void *)(EIGHT_MB - process_num*0x2000), &pcb, sizeof(pcb));

  /* Set TSS values */
  tss.esp0 = EIGHT_MB - (process_num - 1)*0x2000;
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
  pcb_t* pcb_start = get_pcb_add();
	file_desc curr_file = pcb_start->fdt[fd];

	if(curr_file.flags == -1){
		sti();
		return 0;
	}

	if(curr_file.jump_ptr->read != NULL){
		sti();
		return curr_file.jump_ptr->read(fd, buf, nbytes);
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

	if(curr_file.flags == -1){
		sti();
		return -1;
	}

	if(curr_file.jump_ptr->write != NULL){
		sti();
		return curr_file.jump_ptr->write(fd, buf, nbytes);
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
  if(strncmp((int8_t*)filename, (int8_t*)"stdin", strlen((int8_t*)filename)) == 0){
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

  pcb_t* pcb_start = get_pcb_add();

	int i;
	for(i = 2; i < 8; i++){
		if(pcb_start->fdt[i].flags == -1){
			if(strncmp((int8_t*)filename, (int8_t*)"rtc", strlen((int8_t*)filename)) == 0){
				pcb_start->fdt[i].jump_ptr = &rtc_table;
				rtc_open((uint8_t*) filename);
			}
			else if(strncmp((int8_t*)filename, (int8_t*)".", strlen((int8_t*)filename)) == 0){
				pcb_start->fdt[i].jump_ptr = &dir_table;
				dir_open((uint8_t*) filename);
			}
			else{
				pcb_start->fdt[i].jump_ptr = &file_table;
				file_open((uint8_t*) filename);
			}
      pcb_start->fdt[i].flags = 1;
      pcb_start->fdt[i].inode = temp_dentry.inode_num;
      pcb_start->fdt[i].file_position = 0;
      return i;
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
	if(pcb_start->fdt[fd].flags==1){
		if(pcb_start->fdt[fd].jump_ptr->close != NULL){
			pcb_start->fdt[fd].jump_ptr->close(fd);
			pcb_start->fdt[fd].flags = -1;
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

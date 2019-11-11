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
#define MAX_PROG_SIZE FOUR_MB - PROG_OFFSET
#define EIGHT_KB 0x2000

/* Function pointers for rtc */
jump_table rtc_table = {rtc_write, rtc_read, rtc_open, rtc_close};

/* Function pointers for file */
jump_table file_table = {file_write, file_read, file_open, file_close};

/* Function pointers for directory */
jump_table dir_table = {dir_write, dir_read, dir_open, dir_close};

/* Function pointers for stdin (only has terminal read) */
jump_table stdin_table = {NULL, terminal_read, terminal_open, terminal_close};

/* function pointers for stdout(only has terminal write) */
jump_table stdout_table = {terminal_write, NULL, terminal_open, terminal_close};

/* process number: 1st process has pid 1, 0 means no processes have been launched */
int32_t process_num = 0;

/*
 * halt
 *    DESCRIPTION: Halt system call, restores parent process paging and data and closes fds
 *    INPUTS: uint8_t status - return value from halt
 *    OUTPUTS: none
 *    RETURN VALUE: Returns to parent execute
 *    SIDE EFFECTS: halt current process, return to parent process(shell)
 */
int32_t halt(uint8_t status){
  /* If shell tries to halt, just launch shell again */
  if(process_num == 1){
    process_num = 0;
    uint8_t sh[] = "shell";
    execute((uint8_t*)sh);
  }
  //else we are in a child process
  int i; /* Loop variable */
  /* Close all files in the pcb */
  for(i = 0; i < 8; i++){
    close(i);
  }

  process_num--; /* Decrement process number */

  /* Remap user program paging back to parent program */
  set_page_dir_entry(USER_PROG, EIGHT_MB + (process_num - 1)*FOUR_MB);

  /* Flush tlb */
  asm volatile ("      \n\
     movl %%cr3, %%eax \n\
     movl %%eax, %%cr3"
     :
     :
     : "eax"
  );

  pcb_t* cur_pcb = get_pcb_add(); /* Get the current process pcb */
  cur_pcb->process_state=STOPPED; /* Set process state to stopped */
  tss.esp0 = cur_pcb->parent_esp; /* Set TSS esp0 back to parent stack pointer */

  /*
  inline assembly:
    1st line: zero extend bl and move value into eax so that the correct status is returned from halt
    2nd line: set ebp to parent process ebp
    3rd andd 4th line: jump back to parent's system call linkage
  */
  asm volatile ("      \n\
     movzbl %%bl,%%eax \n\
     movl %0,%%ebp     \n\
     leave             \n\
     ret"
     :
     : "r"(cur_pcb->parent_ebp)
     : "ebp","eax"
  );
  return 0;
}

/*
 * execute
 *    DESCRIPTION: Loads executable into memory and returns to user program
 *    INPUTS: const uint8_t* command - command to execute
 *    OUTPUTS: none
 *    RETURN VALUE: -1 for failure, 0 to 255 for success, 256 for an exception
 *    SIDE EFFECTS: Copies executable program and pcb into memory
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
    sti();
    return -1;
  }

  /* Read the executable */
  uint8_t ELF_buf[30];
  uint32_t size; /* Size of executable file */
  if((size = read_data(file_dentry.inode_num, 0, ELF_buf, 30)) == -1){
    /* Return failure */
    sti();
    return -1;
  }

  uint8_t elf[] = "ELF"; /* ELF string to compare */
  /* Check if file is an executable */
  if(!(ELF_buf[0] == 0x7F && !strncmp((int8_t*)(ELF_buf + 1), (int8_t*)elf, 3))){
    /* Return failure */
    sti();
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

  /* Copy executable to 128MB */
  if((size = read_data(file_dentry.inode_num, 0, (uint8_t*)(USER_PROG + PROG_OFFSET), MAX_PROG_SIZE)) == -1){
    /* Return failure */
    return -1;
  }

  /* Create pcb */
  pcb_t pcb;
  pcb.pid = process_num;
  /* Load stdin and stdout jump table and mark as in use */
  pcb.fdt[0].jump_ptr = &stdin_table;
  pcb.fdt[1].jump_ptr = &stdout_table;
  pcb.fdt[0].flags = 1;
  pcb.fdt[1].flags = 1;
  pcb.process_state = 0;

  /* Mark remaining file descriptors as not in use */
  for(i = 2; i < 8; i++){
    pcb.fdt[i].flags = -1;
  }

  /* Set parent esp and ebp for child processes */
  if(process_num >= 2){
    pcb.parent_esp = EIGHT_MB - (process_num-2)*EIGHT_KB;
    asm volatile("  \n\
    movl %%ebp, %0"
    : "=r"(pcb.parent_ebp)
    );
  }

  /* Place pcb in kernel memory */
  memcpy((void *)(EIGHT_MB - process_num*EIGHT_KB), &pcb, sizeof(pcb));

  /* Set TSS to point to kernel stack */
  tss.esp0 = EIGHT_MB - (process_num - 1)*EIGHT_KB;
  tss.ss0 = KERNEL_DS;


  /* Get address of first instruction */
  uint32_t program_addr = *(uint32_t*)(ELF_buf + 24);

  /* Put program_addr into ecx and jump to the context switch */
  asm volatile("       \n\
    movl %0, %%ecx     \n\
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
 *    DESCRIPTION: Reads at a given file descriptor
 *    INPUTS: int32_t fd - file descriptor to read
 *            void* buf - buffer to read to
 *            int32_t nbytes - number of bytes to read
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes){
  cli(); /* Mask interrupts */

  /* Check for a valid fd */
	if(fd < 0 || fd > 7){
		sti();
    /* Return failure */
		return -1;
	}

  /* Pointer to current pcb */
  pcb_t* pcb_start = get_pcb_add();
	file_desc curr_file = pcb_start->fdt[fd];

  /* Check if file descriptor is in use */
	if(curr_file.flags == -1){
		sti();
    /* Return failure */
		return -1;
	}

  /* Check for read in jump table */
	if(curr_file.jump_ptr->read != NULL){
		sti();
    /* Jump to read */
		return curr_file.jump_ptr->read(fd, buf, nbytes);
	}

	sti();

  /* Return success */
	return -1;
}

/*
 * write
 *    DESCRIPTION: Writes to a given file descriptor
 *    INPUTS: int32_t fd - file descriptor to write to
 *            const void* buf - buffer to write from
 *            int32_t nbytes - number of bytes to write
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
  cli(); /* Mask interrupts */

  /* Check for a valid fd */
	if(fd < 0 || fd > 7){
		sti();
    /* Return failure */
		return -1;
	}

  /* Pointer to current pcb */
  pcb_t* pcb_start = get_pcb_add();
	file_desc curr_file = pcb_start->fdt[fd];

  /* Check if descriptor is in use */
	if(curr_file.flags == -1){
		sti();
    /* Return failure */
		return -1;
	}

  /* Check for write in jump table */
	if(curr_file.jump_ptr->write != NULL){
		sti();
    /* Jump to write */
		return curr_file.jump_ptr->write(fd, buf, nbytes);
	}


	sti(); /* Restore interrupts */

  /* Return failure */
	return -1;
}

/*
 * open
 *    DESCRIPTION: Creates a new file descriptor in the pcb
 *    INPUTS: const uint8_t* filename - file to open
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t open(const uint8_t* filename){
  /* Check for NULL */
  if(filename == NULL || strncmp((int8_t*)filename, (int8_t*)"", strlen((int8_t*)filename)) == 0){
   /* Return failure */
	 return -1;
  }

  /* Open stdin */
  if(strncmp((int8_t*)filename, (int8_t*)"stdin", strlen((int8_t*)filename)) == 0){
		terminal_open(filename);
    /* Return fd */
		return 0;
	}

  /* Open stdout */
	if(strncmp((int8_t*)filename, (int8_t*)"stdout", strlen((int8_t*)filename)) == 0){
		terminal_open(filename);
    /* Return fd */
		return 1;
	}


	dentry_t temp_dentry;
	int temp = read_dentry_by_name((uint8_t*)filename, &temp_dentry);

	if(temp == -1){
		return -1;
	}

  /* Pointer to the current pcb */
  pcb_t* pcb_start = get_pcb_add();

	int i; /* Loop variable */
  /* Go through each file descriptor */
	for(i = 2; i < 8; i++){
    /* Find file descriptor not in use */
		if(pcb_start->fdt[i].flags == -1){
      /* Load rtc jump table */
			if(strncmp((int8_t*)filename, (int8_t*)"rtc", strlen((int8_t*)filename)) == 0){
				pcb_start->fdt[i].jump_ptr = &rtc_table;
				rtc_open((uint8_t*) filename);

			} /* Load directory jump table */
      else if(strncmp((int8_t*)filename, (int8_t*)".", strlen((int8_t*)filename)) == 0){
				pcb_start->fdt[i].jump_ptr = &dir_table;
				dir_open((uint8_t*) filename);
			} /* Load file jump table */
      else{
				pcb_start->fdt[i].jump_ptr = &file_table;
				file_open((uint8_t*) filename);
			}
      /* Mark file descriptor as in use and intialize contents */
      pcb_start->fdt[i].flags = 1;
      pcb_start->fdt[i].inode = temp_dentry.inode_num;
      pcb_start->fdt[i].file_position = 0;

      /* Return index */
      return i;
		}
	}

  /* Return failure */
	return -1;
}

/*
 * close
 *    DESCRIPTION: Closes a file descriptor in the pcb
 *    INPUTS: int32_t fd - file descriptor to close
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t close(int32_t fd){
  /* Get a pointer to the pcb */
  pcb_t* pcb_start = get_pcb_add();

  /* Check for an invalid fd */
  if(fd > 7 || fd < 2){
    /* Return failure */
		return -1;
	}
  //generate page exception for testing
  //if(pcb_start->fdt[fd].flags){

  /* Check if file desciptor is in use */
	if(pcb_start->fdt[fd].flags == 1){
    /* Jump to file's close and mark as not in use */
    pcb_start->fdt[fd].jump_ptr->close(fd);
    pcb_start->fdt[fd].flags = -1;
    /* Return success */
		return 0;
	}

  /* Return failure */
	return -1;
}

/*
 * get_pcb_add
 *    DESCRIPTION: Gets a pointer of the current process' pcb
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: A pcb_t pointer
 *    SIDE EFFECTS: none
 */
pcb_t* get_pcb_add(){
  int32_t esp; /* esp value */
  pcb_t* pcb_add; /* address of the pcb */

  /* Get current esp value */
  asm volatile("\n\
    movl %%esp, %0"
    : "=r" (esp)
  );

  /* 8kB = 2^13, so mask everything below 13th bit */
  pcb_add = (pcb_t*)(esp & 0xFFFFE000);

  /* Return pointer */
  return pcb_add;
}

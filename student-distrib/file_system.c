#include "file_system.h"
#include "lib.h"

#define FOUR_KB 4096
#define NAME_LENGTH

/* Directory entry struct */
struct dentry_struct{
  uint8_t file_name[NAME_LENGTH];
  uint32_t file_type;
  uint32_t inode_num;
  uint32_t reserved24[6];
};

/* Boot block struct */
struct boot_struct{
  uint32_t num_dentries;
  uint32_t num_inodes;
  uint32_t num_dblocks;
  uint32_t reserved52[13];
  dentry_t dentries[63];
};

static boot_block_t* boot_block;
static uint32_t* fs_start;
static uint32_t dentry_index = -1;
static uint32_t file_offset = 0;
static uint32_t cur_inode = -1;

/*
 * file_system_init
 *    DESCRIPTION: Initializes the file system
 *    INPUTS: uint32_t* file_sys_start - a pointer to the boot block in memory
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Adds a boot_block_t pointer to the start of the file system
 */
void file_system_init(uint32_t* file_sys_start){
  boot_block = (boot_block_t*)file_sys_start;
  fs_start = file_sys_start;
}

/*
 * find_dentry
 *    DESCRIPTION: Finds a dentry given a filename
 *    INPUTS: const uint8_t* filename - Name of the dentry to find
 *    OUTPUTS: none
 *    RETURN VALUE: dentry_t* - a pointer to the found dentry
 *    SIDE EFFECTS: none
 */
dentry_t* find_dentry(const uint8_t* filename){
  int i;
  for(i = 0; i < boot_block->num_dentries; i++){
    if(strncmp((const int8_t*)(boot_block->dentries[i].file_name), (const int8_t*)filename, NAME_LENGTH) == 0){
      return &(boot_block->dentries[i]);
    }
  }
  return NULL;
}

/*
 * read_dentry_by_name
 *    DESCRIPTION: Copies the dentry by filename
 *    INPUTS: const uint8_t* filename - Name of the dentry to copy
 *            dentry_t* dentry - dentry to copy to
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success or -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t read_dentry_by_name(const uint8_t* filename, dentry_t* dentry){
  dentry_t *file_dentry = find_dentry(filename);
  if(file_dentry == NULL){
    return -1;
  }

  // set dentry = file_dentry
  strncpy((int8_t*)dentry, (int8_t*)file_dentry, NAME_LENGTH); //copy file name, FIX MAGIC NUM
  dentry->file_type = file_dentry->file_type;
  dentry->inode_num = file_dentry->inode_num;

  return 0;
}

/*
 * read_dentry_by_index
 *    DESCRIPTION: Copies a dentry by the given index
 *    INPUTS: uint32_t index - the index of the dentry to copy
 *            dentry_t* dentry - the dentry to copy to
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success or -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
  if(index >= boot_block->num_dentries){
    //index is greater than the number of files in directory, return -1 for FAIL
    return -1;
  }
  dentry_t* file_dentry = &(boot_block->dentries[index]);
  // set dentry = file_dentry
  memcpy(dentry, file_dentry, sizeof(dentry_t));

  return 0;
}

/*
 * read_data
 *    DESCRIPTION: Reads the data of a file
 *    INPUTS: uint32_t inode - the file's inode number
 *            uint32_t offset - how far in the file to start reading
 *            uint8_t* buf - where to copy the data to
 *            uint32_t length - how many bytes to read
 *    OUTPUTS: none
 *    RETURN VALUE: The number of bytes read, or -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
  if(inode >= (boot_block->num_inodes)){
    return -1;
  }
  uint8_t* inode_addr = (uint8_t*)fs_start+(FOUR_KB * (inode+1));
  uint32_t file_size;
  memcpy(&file_size,inode_addr,4); //get file size

  if(offset > file_size){
    //offset is outside of file range
    return -1;
  }

  uint32_t end = (offset+length)>file_size?file_size:offset+length;
  uint32_t dblock_num;
  uint32_t cur = offset;
  uint8_t* num_addr = inode_addr + 4 + (4*(offset/FOUR_KB));
  memcpy(&dblock_num, num_addr, 4);
  uint8_t* dblock_addr = (uint8_t*)fs_start + (boot_block->num_inodes + 1)*FOUR_KB + (dblock_num*FOUR_KB);
  uint32_t copied_length =0;

  while(cur != end){
    uint32_t copyLength = (end - cur) > FOUR_KB?FOUR_KB - (cur % FOUR_KB):(end-cur);
    memcpy(buf+copied_length, dblock_addr+ (cur % FOUR_KB), copyLength);
    cur += copyLength;
    copied_length+=copyLength;
    num_addr += 4;
    memcpy(&dblock_num, num_addr, 4);
    //printf("dblock_num: %d\n", dblock_num);
    dblock_addr = (uint8_t*)fs_start + (boot_block->num_inodes + 1)*FOUR_KB + (dblock_num*FOUR_KB);
  }

  if((offset+length) > file_size){
    length -= (offset+length) - file_size;
  }

  return length;
}

/*
 * file_open
 *    DESCRIPTION: Opens a file and stores the file info
 *    INPUTS: const uint8_t* filename - the file to open
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: Initializes the file offset to 0 and stores the current inode number
 */
int32_t file_open(const uint8_t* filename){
  dentry_t file_dentry;
  read_dentry_by_name(filename, &file_dentry);
  file_offset = 0;
  cur_inode = file_dentry.inode_num;
  return 0;
}

/*
 * file_close
 *    DESCRIPTION: Closes a file
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: Sets the current inode number to -1 and offset to 0
 */
int32_t file_close(void){
  cur_inode = -1;
  file_offset = 0;
  return 0;
}

/*
 * file_read
 *    DESCRIPTION: Reads the data in a file to a buffer
 *    INPUTS: int32_t fd - file to read
 *            void* buf - the buffer to copy to
 *            uint32_t num_bytes - the number of bytes to read
 *    OUTPUTS: none
 *    RETURN VALUE: Number of bytes read
 *    SIDE EFFECTS: Updates the current file offset
 */
int32_t file_read(int32_t fd, void* buf, uint32_t num_bytes){
  if(cur_inode == -1){
    return -1;
  }
  int read_ret = 0;
  if((read_ret = read_data(cur_inode, file_offset, (uint8_t*)buf, num_bytes)) == -1){
    return -1;
  }
  file_offset += num_bytes;
	return read_ret;
}

/*
 * file_write
 *    DESCRIPTION: Writes to a file
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t file_write(void){
  /* File system is read only, so return failure */
  return -1;
}

/*
 * dir_open
 *    DESCRIPTION: Opens the directory
 *    INPUTS: const uint8_t* filename - name of the directory to open
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: Sets the current index of the dentry to 0
 */
int32_t dir_open(const uint8_t* filename){
  dentry_t dentry;
  dentry_index = 0;
  read_dentry_by_name(filename, &dentry);
  return 0;
}

/*
 * dir_close
 *    DESCRIPTION: Close the directory
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success
 *    SIDE EFFECTS: Sets dentry index to -1 to indicate that it's closed
 */
int32_t dir_close(void){
  /* Set dentry index to an invalid number */
  dentry_index = -1;

  /* Return success */
  return 0;
}

/*
 * dir_read
 *    DESCRIPTION: Read a file name in the directory
 *    INPUTS: int32_t fd - directory to read from
 *            void* buf - buffer to read to
 *            nbytes - number of bytes the buffer can hold
 *    OUTPUTS: none
 *    RETURN VALUE: 0 for success, -1 for failure
 *    SIDE EFFECTS: Increments the dentry index to the next file
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
  dentry_t dentry;

  if(dentry_index == -1){
    return -1;
  }
  if(dentry_index >= boot_block->num_dentries){
    return 0;
  }
	if(-1 == read_dentry_by_index(dentry_index, &dentry)){
    return -1;
  }
  //write file name to buf
  strncpy((int8_t*)buf, (const int8_t*)dentry.file_name, NAME_LENGTH);
  dentry_index++;
  return NAME_LENGTH;
}

/*
 * dir_write
 *    DESCRIPTION: Write to the directory
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: -1 for failure
 *    SIDE EFFECTS: none
 */
int32_t dir_write(void){
  /* File system is read only, so return failure */
  return -1;
}

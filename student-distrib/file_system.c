#include "file_system.h"
#include "lib.h"

#define FOUR_KB 4096

// directory entry struct
struct dentry_struct{
  uint8_t file_name[32];
  uint32_t file_type;
  uint32_t inode_num;
  uint32_t reserved24[6];
};

// boot block
struct boot_struct{
  uint32_t num_dentries;
  uint32_t num_inodes;
  uint32_t num_dblocks;
  uint32_t reserved52[13];
  dentry_t dentries[63];
};

static boot_block_t boot_block;
static uint32_t* fs_start;//addr of file system start

void file_system_init(uint32_t* file_sys_start){
  memcpy(&boot_block, file_sys_start, FOUR_KB);
  fs_start = file_sys_start;
}

dentry_t* find_dentry(const uint8_t* filename){
  int i;
  for(i=0;i<boot_block.num_dentries;i++){
    if(strncmp((const int8_t*)(boot_block.dentries[i].file_name),(const int8_t*)filename,32)==0)return &(boot_block.dentries[i]);
  }
  return NULL;
}

int32_t read_dentry_by_name(const uint8_t* filename, dentry_t* dentry){
  dentry_t *file_dentry = find_dentry(filename);
  if(file_dentry==NULL)return -1;

  // set dentry = file_dentry
  strncpy((int8_t*)dentry, (int8_t*)file_dentry, 32); //copy file name, FIX MAGIC NUM
  dentry->file_type = file_dentry->file_type;
  dentry->inode_num = file_dentry->inode_num;
  return 0;
}

int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
  if(index>=boot_block.num_dentries)return -1; //index is greater than the number of files in directory, return -1 for FAIL
  dentry_t *file_dentry = &boot_block.dentries[index];
  // set dentry = file_dentry
  strncpy((int8_t*)dentry, (int8_t*)file_dentry, 32); //copy file name, FIX MAGIC NUM
  dentry->file_type = file_dentry->file_type;
  dentry->inode_num = file_dentry->inode_num;
  return 0;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
  if(inode >=boot_block.num_dentries) return -1;
  uint32_t* inode_addr = (fs_start)+(FOUR_KB * (inode+1));
  uint32_t file_size;
  memcpy(&file_size,inode_addr,4); //get file size
  if(offset>file_size)return -1;

  return 0;
}

int32_t file_open(){
  return 0;
}

int32_t file_close(){
  return 0;
}

int32_t file_read(void* buf, uint32_t num_bytes){
  int read_ret=0;
  // if((read_ret = read_data(, , (uint8_t*)buf, num_bytes)) == -1){
  //   return -1;
  // }
	return read_ret;
}

int32_t file_write(){
  return -1;
}

int32_t dir_open(const uint8_t* filename){
  dentry_t dentry;
  read_dentry_by_name(filename, &dentry);
  return 0;
}

int32_t dir_close(){
  return 0;
}

int32_t dir_read(){
	// read_dentry_by_index(???);
  return 0;
}

int32_t dir_write(){
  return -1;
}

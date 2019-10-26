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

static boot_block_t* boot_block;
static uint32_t* fs_start;//addr of file system start
static uint32_t dentry_index;
static uint32_t file_offset;
static uint32_t cur_inode;

void file_system_init(uint32_t* file_sys_start){
  boot_block = (boot_block_t*)file_sys_start;
  fs_start = file_sys_start;
}

dentry_t* find_dentry(const uint8_t* filename){
  int i;
  for(i=0;i<boot_block->num_dentries;i++){
    if(strncmp((const int8_t*)(boot_block->dentries[i].file_name),(const int8_t*)filename,32)==0)return &(boot_block->dentries[i]);
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
  if(index>=boot_block->num_dentries)return -1; //index is greater than the number of files in directory, return -1 for FAIL
  dentry_t *file_dentry = &(boot_block->dentries[index]);
  // set dentry = file_dentry
  memcpy(dentry,file_dentry,sizeof(dentry_t));
  return 0;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
  if(inode >= (boot_block->num_dentries-1) ) return -1;
  uint32_t* inode_addr = (fs_start)+(FOUR_KB * (inode+1));
  uint32_t file_size;
  memcpy(&file_size,inode_addr,4); //get file size

  if(offset > file_size)
    return -1; //offset is outside of file range

  uint32_t end=(offset+length)>file_size?file_size:offset+length;
  uint32_t dblock_num;
  uint32_t cur = offset;
  uint32_t* num_addr = inode_addr + 4 +(4*(offset/FOUR_KB));
  memcpy(&dblock_num, num_addr,4);

  uint32_t* dblock_addr = fs_start + (boot_block->num_inodes - 1)*FOUR_KB + (dblock_num*FOUR_KB);
  uint32_t copied_length =0;
  while(cur != end){
    uint32_t copyLength = (end - cur) > FOUR_KB?FOUR_KB - (cur % FOUR_KB):(end-cur);
    memcpy(buf+copied_length, dblock_addr+ (cur % FOUR_KB), copyLength);
    cur += copyLength;
    copied_length+=copyLength;
    num_addr += 4;
    memcpy(&dblock_num, num_addr, 4);
    dblock_addr = fs_start + (boot_block->num_inodes - 1)*FOUR_KB + (dblock_num*FOUR_KB);

  }

  if((offset+length) > file_size){
    length -= (offset+length) - file_size;
  }

  return length;
}

int32_t file_open(const uint8_t* filename){
  dentry_t file_dentry;
  read_dentry_by_name(filename,&file_dentry);
  file_offset = 0;
  cur_inode = file_dentry.inode_num;
  return 0;
}

int32_t file_close(){
  cur_inode = -1;
  file_offset=0;
  return 0;
}

int32_t file_read(int32_t fd, void* buf, uint32_t num_bytes){
  if(cur_inode==-1)return -1;
  int read_ret = 0;
  if((read_ret = read_data(cur_inode, file_offset, (uint8_t*)buf, num_bytes)) == -1){
    return -1;
  }
  file_offset+=num_bytes;
	return read_ret;
}

int32_t file_write(){
  return -1;
}

int32_t dir_open(const uint8_t* filename){
  dentry_t dentry;
  dentry_index = 1;
  read_dentry_by_name(filename, &dentry); //doesn't do anything yet...
  return 0;
}

//does nothing
int32_t dir_close(){
  return 0;
}

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes){
  dentry_t dentry;
  if(dentry_index >= boot_block->num_dentries) return 0;
	if(-1==read_dentry_by_index(dentry_index, &dentry))return -1;
  strncpy((int8_t*)buf, (const int8_t*)dentry.file_name,32);//write file name to buf
  dentry_index++;
  return strlen((const int8_t*)buf);
}

int32_t dir_write(){
  return -1;
}

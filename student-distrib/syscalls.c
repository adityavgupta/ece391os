#include "syscalls.h"

#include "lib.h"

int32_t halt(uint8_t status){
  return 0;
}

int32_t execute(const uint8_t* command){
	uint8_t filename[32];
  int i=0;
  while(command[i]!='\0' && command[i]!=' ' && i<32){
    filename[i]=command[i];
    i++;
  }
  filename[i]='\0';
  dentry_t file_dentry;
  //invalid file name
  if(read_dentry_by_name(filename,&file_dentry)==-1)return -1;
  uint8_t ELF_buf[4];
  read_data(file_dentry.inode_num, 0, ELF_buf, 4);
  uint8_t elf[]="ELF";
  //not an executable
  if(! (ELF_buf[0]==0x7F && !strncmp((int8_t*)(ELF_buf+1),(int8_t*)elf,3)) )return -1; //check for ELF magic number
  return 69;
}

int32_t read(int32_t fd, void* buf, int32_t nbytes){
  return 0;
}

int32_t write(int32_t fd, void* buf, int32_t nbytes){
  return 0;
}

int32_t open(const uint8_t* filename){
  return 0;
}

int32_t close(int32_t fd){
  return 0;
}

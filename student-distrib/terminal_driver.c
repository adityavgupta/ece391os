#include "terminal_driver.h"
#include "lib.h"
#define VIDEO       0xB8000

//extern unsigned char buf[128];
unsigned char buf[128]={0}; // Buffer of size that is 128;
volatile int is_finished;
int buf_index;

int str_len_count;
void clear_buf(void){
		int buf_counter;
		
		for(buf_counter=0; buf_counter<=128;buf_counter++){
				buf[buf_counter]=0;
		}
		
		str_len_count=0;
		buf_index=0;
		is_finished=0;
}

void open(void){
		str_len_count=0;
		buf_index=0;
		is_finished=0;
}

int read(unsigned char* copy_buf,int nbytes){
		str_len_count=0;
		is_finished=0;
		buf_index=0;
		while(is_finished==0){
			
		}
		cli(); //prevents keyboard interrupts from occurring between the buffer being written
		if(str_len_count>=128){ //Sanity Check
				str_len_count=127;
		}
		
		strncpy((char*)copy_buf,(char*)buf,str_len_count+1);
		clear_buf();
		sti(); 
		
		return str_len_count;
		
}

int min(int a,int b){
	return a<b?a:b;
}

void keyboard_helper(uint8_t scan_code){

				if(buf_index>=128){
					is_finished=1;
					new_line();
					clear_buf();
				}
				else if(scan_code=='\n'){
					//printf("\n");
					new_line();
					buf[buf_index]=scan_code;
					buf_index++;
					clear_buf();
				}
				 else{
					putc(scan_code);
					buf[buf_index]=scan_code;
					buf_index++;
				}


}

int write(unsigned char* copy_buf,int nbytes){
	
	int strlength=strlen(copy_buf);
	
	int i=0;
	for(i=0;i<strlength;i++){
		keyboard_helper(copy_buf[i]);
	}
	
	return -1;
	
}


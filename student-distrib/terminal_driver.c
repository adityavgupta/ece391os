#include "terminal_driver.h"
#include "lib.h"
#include "kb.h"
#define VIDEO       0xB8000

//extern unsigned char buf[128];
volatile unsigned char buf[128]={0}; // Buffer of size that is 128;
volatile unsigned char prev_buf[128];
volatile int is_finished;
int volatile buf_index;
int volatile prev_index;
int flag_1=0;

int str_len_count;
void clear_buf(void){
		int buf_counter; 
		
		for(buf_counter=0; buf_counter<=128;buf_counter++){
				prev_buf[buf_counter]=buf[buf_counter];
				buf[buf_counter]=0;
		}
		prev_index=buf_index;
		str_len_count=0;
		buf_index=0;
		//is_finished=0;
}

void open(void){
		str_len_count=0;
		buf_index=0;
		is_finished=0;
}



void keyboard_helper(uint8_t scan_code){

			
				if(buf_index>=128){
					//is_finished=1;
					clear_buf();
				}
				if(scan_code=='\n'){
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
					if(x_is_zero()){
						new_line();

					}
				}


}

int write(unsigned char* copy_buf,int nbytes){
	
	int strlength=nbytes/sizeof(unsigned char);
	
	int i=0;
	for(i=0;i<strlength;i++){
		keyboard_helper(copy_buf[i]);
	}
	
	
	return -1;
	
}

void close(void){
		int i;
		for(i=0;i<128;i++){
			buf[i]=0; // Buffer of size that is 128;
		}
		is_finished=0;
		buf_index=0;
	
}


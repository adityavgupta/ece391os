#include "kb.h"
#include "x86_desc.h"
#include "i8259.h"
#include "lib.h"

#define IRQ_NUM 1
#define RECENT_RELEASE 0x80
#define LEFT_SHIFT 42
#define RIGHT_SHIFT 54
#define CAPS_LOCK 58
#define NEW_LINE 28
#define BACK_SPACE 14
#define CTRL 29
#define L_CHAR 38



//extern unsigned char buf[128];
int flag=0;
unsigned long flags; /* Hold current flags */

static volatile int fuck_me=0;
//extern int flag_1;

volatile unsigned char buf[128]={0}; // Buffer of size that is 128;
volatile unsigned char prev_buf[128];
volatile int is_finished=0;
int volatile buf_index=0;
int volatile prev_index=0;
int flag_1=0;

int str_len_count;

unsigned char shift_pressed = 0;
unsigned char caps_lock=0;
unsigned char ctrl_pressed=0;
unsigned char kbdus[256] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
  '9', '0', '-', '=', '\b',	/* Backspace */
  '\t',			/* Tab */
  'q', 'w', 'e', 'r',	/* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
 '\'', '`',   0,		/* Left shift */
 '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
  'm', ',', '.', '/',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
	    0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
  '(', ')', '_', '+', '\b',	/* Backspace */
  '\t',			/* Tab */
  'Q', 'W', 'E', 'R',	/* 19 */
  'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
    0,			/* 29   - Control */
  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
 '"', '~',   0,		/* Left shift */
 '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
  'M', '<', '>', '?',   0,				/* Right shift */
  '*',
    0,	/* Alt */
  ' ',	/* Space bar */
    0,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* Scroll Lock */
    0,	/* Home key */
    0,	/* Up Arrow */
    0,	/* Page Up */
  '-',
    0,	/* Left Arrow */
    0,
    0,	/* Right Arrow */
  '+',
    0,	/* 79 - End key*/
    0,	/* Down Arrow */
    0,	/* Page Down */
    0,	/* Insert Key */
    0,	/* Delete Key */
    0,   0,   0,
    0,	/* F11 Key */
    0,	/* F12 Key */
    0,	/* All other keys are undefined */
};


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



int read(unsigned char* copy_buf,int nbytes){
		
		//extern volatile int is_finished;

	
		//is_finished=0;
		
		if(copy_buf==NULL){
			return 0;
		}
		
		while(fuck_me==0){

		}
		
		//printf("%c",prev_buf[0]);
		
		//printf("%d %d %d",nbytes,prev_index,buf_index);
		
		if(nbytes< prev_index*sizeof(unsigned char)){
				memcpy(copy_buf,prev_buf,nbytes);
				return nbytes;	
		} else{
				memcpy(copy_buf,prev_buf,prev_index*sizeof(unsigned char));
				return prev_index*sizeof(unsigned char);
		}
		
		
}



/*
 * keyboard_init
 *    DESCRIPTION: Enables keyboard interrupts
 *    INPUTS: none
 *    OUTPUTS: none
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Unmasks IRQ1 for keyboard interrupts on PIC
 */
void keyboard_init(void){
    /* Enable keyboard IRQ on PIC */
		enable_irq(IRQ_NUM);
		buf[0]='\n';
}

/*
 * keyboard_interrupt_handler
 *    DESCRIPTION: Handler for keyboard interrupts
 *    INPUTS: none
 *    OUTPUTS: Character on the screen
 *    RETURN VALUE: none
 *    SIDE EFFECTS: Prints keyboard input to screen, and sends EOI
 */
/*
int should_stop(void){
	
		printf(" %d ",buf_index); 
		if(buf[buf_index]=='\n' || buf_index==128){
				return 1;
		} else{
				//printf("no\n");
				return 0;
		}
}*/
 
void keyboard_interrupt_handler(void){
	//extern static int screen_x;
	//extern static int screen_y;
	extern volatile int is_finished;

    /* Mask interrupts and store flags */
    cli_and_save(flags);
    /* Mask IRQ on PIC */
		disable_irq(IRQ_NUM);

    /* Send EOI to PIC to indicate interrupt is serviced */
		send_eoi(IRQ_NUM);

    /* Read the keyboard data buffer to get the current character */
		uint8_t scan_code = inb(0x60);
		
		if((scan_code>=90 && scan_code<=128)||(scan_code>=(90+128))){
			return;	
		}
		
		
		
		//printf("%d\n",scan_code);

    /* Test if the character is to be printed or not */
		if(scan_code < RECENT_RELEASE){
        /* If the shift key is pressed adds 128 to index into capital characters 128 is the for the additional 128 character when they are capitalized */
				//putc(kbdus[scan_code]);
        /* Checks to see if the shift key has been recently released or not */
				//printf(" %d ",scan_code);
				
				if(buf_index>=128){
					fuck_me=1;
					clear_buf();
				}
				
				if(scan_code==CTRL){
						ctrl_pressed=1;
				} else if(scan_code==L_CHAR && ctrl_pressed==1){
					reset_screen();
					clear_buf();
					clear();
				}else if(scan_code== BACK_SPACE){ 
					
					int temp=back_space();
					
					if(temp==0){
					buf[buf_index]=0;
					buf_index--;
					if(buf_index<0){
							buf_index=0;
					}
					}
				}else if(scan_code==NEW_LINE){
					//printf("\n");
					new_line();
					buf[buf_index]=kbdus[NEW_LINE];
					buf_index++;
					fuck_me=1;
					//printf("\n%d\n",is_finished);
					clear_buf();
				}
				else if(scan_code == (LEFT_SHIFT) || scan_code == (RIGHT_SHIFT)){
          /* Sets to shift pressed to be pressed 1 is used ot indicate an on state */
					shift_pressed = 1;
					//printf(" Shift has been pressed ");
				} else if(scan_code==CAPS_LOCK){
					caps_lock=(caps_lock+1)%2;
					//printf(" %d ",caps_lock);
				}else if((caps_lock==1) != (shift_pressed==1)){
					putc(kbdus[scan_code+90]); //90 is the offset required to obtain the capital letters
					buf[buf_index]=kbdus[scan_code+90];
					buf_index++;
				} else{
					putc(kbdus[scan_code]);
					buf[buf_index]=kbdus[scan_code];
					buf_index++;
					if(x_is_zero()){
						new_line();

					}
				}
		} else{
      /* If the key is recntly released gets rid of the shift_pressed flag */
			if(scan_code == LEFT_SHIFT+RECENT_RELEASE || scan_code == RIGHT_SHIFT+RECENT_RELEASE ){
        /* 0 is used ot indicate a off state */
				shift_pressed = 0;
				//printf(" Shift has been released ");
			} else if(scan_code==CTRL+RECENT_RELEASE){
					ctrl_pressed=0;
					
			}
		}

    /* Allow interrupts again */
		restore_flags(flags);
    /* Unmask the IRQ1 on the PIC */
		enable_irq(IRQ_NUM);
}

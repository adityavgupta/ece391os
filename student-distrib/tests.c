#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "kb.h"
#include "file_system.h"

#define SYSCALL_NUM 0x80
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/*
 * idt_test_2
 *		ASSERTS: Reserved entries are correct
 *		INPUTS: None
 *    OUTPUTS: PASS/FAIL
 *		SIDE EFFECTS: None
 *		COVERAGE: IDT entries
 *		FILES: x86_desc.h
 */
int idt_test_2(){
  TEST_HEADER;

  int result=PASS;
  int i;
  for(i=0;i<32;i++){ //32 is used to indicate the number of exceptions that are present in the IDT table
    if(!(idt[i].reserved0==0 &&
         idt[i].reserved1==1 &&
         idt[i].reserved2==1 &&
         idt[i].reserved3==1 &&
         idt[i].reserved4==0 )){ //This idicates the field 01110000000 if it is not true asserts a failure
      	assertion_failure();
      result=FAIL;
    }
  }

  for(i=32;i<256;i++){ // 32 is used to indicate the number of exceptions in the idt talbe while 256 is the number of values that are in the table
    if(!(idt[i].reserved0==0 &&
         idt[i].reserved1==1 &&
         idt[i].reserved2==1 &&
         idt[i].reserved3==0 &&
         idt[i].reserved4==0)){ //This indicates the field 01100000000 if this is not true asserts a failure
      	assertion_failure();
      	result=FAIL;
    }
  }

  return result;
}

/*
 * idt_test_3
 *		ASSERTS: dpl and size are correct
 *		INPUTS: None
 *    OUTPUTS: None
 *		SIDE EFFECTS: None
 *		COVERAGE: IDT entries
 *		FILES: x86_desc.h
 */
int idt_test_3(){
  TEST_HEADER;

  int result=PASS;
  int i;
  for(i=0;i<256;i++){ //256 is the number used to indicate the number of entries in the idt table
    if(!(idt[i].dpl==0 && idt[i].size==1) && i !=SYSCALL_NUM){ //Checks if the priority level is set to 0 and the size is set to 32 bits
      	assertion_failure();
      result=FAIL;
    }
  }

  if(!(idt[SYSCALL_NUM].dpl==3 && idt[SYSCALL_NUM].size==1)){ //checks if the priotiy level is set to 3 and the size is set to 1
    assertion_failure();
    result=FAIL;
  }

  return result;
}

/*
 * page_fault_test0
 *		ASSERTS: Dereferencing NULL causes page fault
 *		INPUTS: None
 *    OUTPUTS: FAIL or Exception
 *		SIDE EFFECTS: Exception
 *		COVERAGE: Paging
 *		FILES: Paging.c
 */
void page_fault_test0(){
	TEST_HEADER;

  unsigned int address = NULL;
  printf("Accessing address: %x\n", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test0", FAIL);
}

/*
 * page_fault_test1
 *		ASSERTS: Page fault for out of bounds memory access
 *		INPUTS: None
 *    OUTPUTS: Exception or FAIL
 *		SIDE EFFECTS: Exception
 *		COVERAGE: Paging
 *		FILES: Paging.c
 */
void page_fault_test1(){
  TEST_HEADER;

  unsigned int address = 0x3FFFFF;
  printf("Accessing address: %x\n", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test1", FAIL);
}

/*
 * page_fault_test2
 *		ASSERTS: Page fault for out of bounds memory access
 *		INPUTS: None
 *    OUTPUTS: Exception or FAIL
 *		SIDE EFFECTS: Exception
 *		COVERAGE: Paging
 *		FILES: paging.c
 */
 void page_fault_test2(){
  TEST_HEADER;

  unsigned int address = 0x800001;
  printf("Accessing address: %x\n", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test2", FAIL);
}

/*
 * page_fault_test3
 *		ASSERTS: Exception when accessing just outside of video memory page
 *		INPUTS: None
 *    OUTPUTS: Exception or FAIL
 *		SIDE EFFECTS: Exception
 *		COVERAGE: Paging
 *		FILES: paging.c
 */
void page_fault_test3(){
  TEST_HEADER;

  unsigned int address = 0xB7FFF;
  printf("Accessing address: %x\n", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test3", FAIL);
}

/*
 * page_fault_test4
 *		ASSERTS: Page fault for accessing memory just outside kernel memory
 *		INPUTS: None
 *    OUTPUTS: Exception or FAIL
 *		SIDE EFFECTS: Exception
 *		COVERAGE: Paging
 *		FILES: paging.c
 */
void page_fault_test4(){
  TEST_HEADER;

  unsigned int address = 0xC0001;
  printf("Accessing address: %x\n", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test3", FAIL);
}

/*
 * page_fault_test5
 *		ASSERTS: Video memory is accessable at beginning address
 *		INPUTS: None
 *    OUTPUTS: Exception or PASS
 *		SIDE EFFECTS:
 *		COVERAGE:
 *		FILES:
 */
void page_fault_test5(){
  TEST_HEADER;

  unsigned int address = 0xB8000;
  printf("Accessing address: %x\n", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test5", PASS);
}

/*
 * page_fault_test6
 *		ASSERTS: Kernel memory is accessable at beginning address
 *		INPUTS: None
 *    OUTPUTS: PASS or Exception
 *		SIDE EFFECTS: Exception
 *		COVERAGE: Paging
 *		FILES: paging.c
 */
void page_fault_test6(){
  TEST_HEADER;

  unsigned int address = 0x400000;
  printf("Accessing address: %x\n", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test6", PASS);
}

/*
 * divide_zero_test
 *		ASSERTS: Arithmetic exception for dividing by zero
 *		INPUTS: None
 *    OUTPUTS: FAIL or Exception
 *		SIDE EFFECTS: Exception
 *		COVERAGE: Exception in IDT
 *		FILES: x86_desc.h and init_idt.c
 */
void divide_zero_test(){
  TEST_HEADER;

  printf("Attempting division 69/0 \n");
  int x = 69;
  int y = 0;
  x = x / y;
  TEST_OUTPUT("divide_zero_test", FAIL);
}

/*
 * page_directory_test
 *		ASSERTS: Page directory has tables marked not present
 *		INPUTS: None
 *    OUTPUTS: PASS or FAIL
 *		SIDE EFFECTS: None
 *		COVERAGE: Paging
 *		FILES: paging.c
 */
int page_directory_test(){
  TEST_HEADER;

  int result = PASS;
  int i;

  if((get_dir(0) & 0x03) == 0x02){
    return FAIL;
  }

  if((get_dir(1) & 0x03) != 0x03){
    return FAIL;
  }

  for(i=2; i<1024; i++){
    if((get_dir(i) & 0x03) != 0x02){
      result = FAIL;
      break;
    }
  }

  return result;
}

/*
 * page_table_test
 *		ASSERTS: Page table has not present pages and correct addresses
 *		INPUTS: None
 *    OUTPUTS: PASS or FAIL
 *		SIDE EFFECTS: None
 *		COVERAGE: Paging
 *		FILES: paging.c
 */
int page_table_test(){
  TEST_HEADER;

  int result=PASS;
  int i;

  for(i=0;i<1024;i++){
		if(i!=((get_page(i))>>12)){
			return FAIL;
		}
    if(i==0xB8){
      if((get_page(i)&0x03)!=0x03){
        return FAIL;
      }
    }
    else if((get_page(i)&0x03)!=0x02){
    	return FAIL;
    }
  }

  return result;
}

/* Checkpoint 2 tests */

void rtc_test(){
	//TEST_HEADER;

	uint8_t buf[1];
	uint32_t count = 0;
	reset_screen();
	rtc_write(0, buf, 14);
	count = 0;
	while(count < 1000000000){
		count++;
	}
	// clear();
	// rtc_write(0, buf, 13);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 12);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 11);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 10);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 9);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 8);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	reset_screen();
	rtc_write(0, buf, 7);
	count = 0;
	while(count < 1000){
		count++;
	}
	// clear();
	// rtc_write(0, buf, 6);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 5);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 4);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
	// clear();
	// rtc_write(0, buf, 3);
	// count = 0;
	// while(count < 1000){
	// 	count++;
	// }
}

void rtc_read_test(){
	uint8_t buf[1];
	rtc_read(0, buf, 0);
}

void buffer_overflow_read(void) {
  open();
  unsigned char string[140];

  int temp = read(string, 140*sizeof(unsigned char));

  putc('\n');
  // the write should just print out 128 characters
  write(string, temp);
  close();


}



void buffer_overflow_write(void) {


  unsigned char string[160];
  int i;
  for(i=0;i<160;i++){
    string[i]='a';
  }

  //string[159]=;
  open();

  write(string,160*sizeof(unsigned char));

  close();
}



void fang_lu_test_5(void){

	open();
	unsigned char string[128];

	int temp=read(string,128*sizeof(unsigned char));

	write(string,temp);
	close();
}


void read_file_test(){
	int8_t buf[10000];
  uint8_t name[] = "hello";
  volatile uint32_t size;
  int i;
  file_open(name);
  if((size = file_read(69, buf, 10000)) == -1){
    printf("Error\n");
  }
  file_close(name);
  buf[10000] = '\0';
  for(i=0; i<size; i++){
    putc(buf[i]);
  }
	//puts(buf);
}

void dir_read_test(){
	uint8_t buf[33];
  int32_t cnt;
  uint8_t testFile[]={'.'};
  dir_open((const uint8_t*)testFile);
  while (0 != (cnt = dir_read (5, (void*)buf, 32))) {
    if(cnt==-1)break;
    puts((int8_t*)buf);
    printf("\n");
  }
  dir_close();
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	// TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
  // TEST_OUTPUT("idt_test2", idt_test_2());
  // TEST_OUTPUT("idt_test3", idt_test_3());
	// page_fault_test0();
	// page_fault_test1();
	// page_fault_test2();
	// page_fault_test3();
	// page_fault_test4();
	// page_fault_test5();
	// page_fault_test6();
  // divide_zero_test();
  // TEST_OUTPUT("page_directory_test", page_directory_test());
 	// TEST_OUTPUT("page_table_test", page_table_test());
 	// rtc_test();
	//rtc_read_test();
  //TEST_OUTPUT("page_directory_test", page_directory_test());
 	//TEST_OUTPUT("page_table_test", page_table_test());
	//fang_lu_test_5();
	//buffer_overflow_read();
	read_file_test();
	//dir_read_test();
}

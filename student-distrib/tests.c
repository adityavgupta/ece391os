#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"

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

// add more tests here
/* At minimum, your tests should over:
 * Values contained in the IDT array - An example has been provided in tests.
 * Receiving an RTC interrupt
 * Interpreting various scancodes in the keyboard handler
 * Values contained in your paging strutures
 * Dereferencing different address ranges with paging turned on
 * Checking bad or garbage input and return values for any function you write
 */

// kernel range
void page_fault_test1(){
  TEST_HEADER;

  unsigned int address = 0x400001;
  printf("Accessing address: %x", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test1", FAIL);
}

// kernel range
void page_fault_test2(){
  TEST_HEADER;

  unsigned int address = 0x800001;
  printf("Accessing address: %x", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test2", FAIL);
}

// video memory range
void page_fault_test3(){
  TEST_HEADER;

  unsigned int address = 0xB7FFF;
  printf("Accessing address: %x", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test3", FAIL);
}

// video memory range
void page_fault_test4(){
  TEST_HEADER;

  unsigned int address = 0xC0001;
  printf("Accessing address: %x", address);
  (*(int*)address) = 0x1;
  TEST_OUTPUT("page_fault_test3", FAIL);
}
//dividing by zero, should cause divide by zero exception
void divide_zero_test(){
  TEST_HEADER;
  printf("Attempting division 69/0 \n");
  int n1=69;
  int n2=0;
  int i=n1/n2;
  TEST_OUTPUT("divide_zero_test", FAIL);
}

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

int page_table_test(){
  TEST_HEADER;
  int result=PASS;
  int i;
  for(i=0;i<1024;i++){
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
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
  TEST_OUTPUT("idt_test2", idt_test_2());
  TEST_OUTPUT("idt_test3", idt_test_3());
	//page_fault_test1();
  //divide_zero_test();
  TEST_OUTPUT("page_directory_test", page_directory_test());
 	//TEST_OUTPUT("page_table_test", page_table_test());
}

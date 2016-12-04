#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "../src/PearlDiver.h"
#include <stdio.h>

void getRandomTrits (int *RandomTrits, int length);
int init_suite(void) {return 0;}
int clean_suite(void) {return 0;}

/************** Test case functions ****************/

void test_case_sample(void) {
	CU_ASSERT(CU_TRUE);
	CU_ASSERT_NOT_EQUAL(2,-1);
}

void test_case_search(void)
{
	PearlDiver pearlDiver = malloc(sizeof(PearlDiver));
}


/*
void getRandomTrits (int *RandomTrits, int length) {
	int i = 0;
	srand(time(NULL));
	while (i < length) {
		RandomTrits[i] = rand() % 3 - 1;
		i++;
	}

}
*/
/* 
   for(i = 0; i < length; i++)
	   printf("%d", RandomTrits[i]);
*/

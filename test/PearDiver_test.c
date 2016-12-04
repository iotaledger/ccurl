#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "../src/PearlDiver.h"
#include <stdio.h>

int init_suite(void) {return 0;}
int clean_suite(void) {return 0;}

/************** Test case functions ****************/

void test_case_sample(void) {
	CU_ASSERT(CU_TRUE);
	CU_ASSERT_NOT_EQUAL(2,-1);
}

void test_case_search(void)
{
	// Remember, it creates a thread.
}

#include "acunit/i.h"
#include "../src/Hash.h"
#include "../src/PearlDiver.h"
//#include "random_trits.h"

#ifdef __APPLE__
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#else
#include "BCUnit/BCUnit.h"
#include "BCUnit/Basic.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



void runtests();
void getRandomTrits (long *RandomTrits, int length);
int init_suites(void) {return 0;}
int clean_suites(void) {return 0;}

/************** Test case functions ****************/


void test_pearl_diver_destroy(void)
{
	PearlDiver *pearl_diver = NULL;

	init_pearldiver(pearl_diver);
	//pearl_diver = malloc(sizeof(struct _PearlDiver));
	
	CU_ASSERT(sizeof(struct _PearlDiver) > 8);

	free(pearl_diver);
}

void test_pearl_diver_search(void)
{
	PearlDiver *pearl_diver = NULL;

	init_pearldiver(pearl_diver);

	if (pearl_diver == NULL) {
		CU_ASSERT(false);
		return;
	}
	int nonce_size = 1;
	long trits[TRANSACTION_LENGTH];// = { trit_1 };
	getRandomTrits(trits, TRANSACTION_LENGTH);

	//clock_t start = clock(), diff;
	search(pearl_diver, trits, TRANSACTION_LENGTH, nonce_size, 32);
	//diff = clock() - start;
	
	//CU_ASSERT(out);
	
	//free(pearl_diver);
}

void test_pearl_diver_threads(void)
{
	PearlDiver *pearl_diver = NULL;
	init_pearldiver(pearl_diver);

	if (pearl_diver == NULL) {
		CU_ASSERT(false);
		return;
	}
	
	long trits[TRANSACTION_LENGTH];// = { trit_1 };
	long expect[TRANSACTION_LENGTH];// = { trit_output_1 };
	//getRandomTrits(trits, TRANSACTION_LENGTH);

	clock_t start = clock(), diff;
	search(pearl_diver, trits, TRANSACTION_LENGTH, 18, 8);
	diff = clock() - start;

	CU_ASSERT(memcmp(trits, expect, TRANSACTION_LENGTH * sizeof(long)));
	CU_ASSERT(diff > 50000000);

	free(pearl_diver);
}

void getRandomTrits (long *RandomTrits, int length) {
	int i = 0;
	srand(time(NULL));
	while (i < length) {
		RandomTrits[i] = rand() % 3 - 1;
		i++;
	}

}

static CU_TestInfo tests[] = {
	//{"PearlDiver Create Test", test_pearl_diver_create},
	{"PearlDiver Destroy Test", test_pearl_diver_destroy},
	{"PearlDiver Search Test", test_pearl_diver_search},
	//{"PearlDiver Thread Test", test_pearl_diver_threads},
	CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
	{ "suitename1", init_suites, clean_suites, NULL, NULL, tests },
	CU_SUITE_INFO_NULL,
};

ADD_SUITE(suites);

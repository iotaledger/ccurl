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
#include <unistd.h>



void runtests();
void getRandomTrits (trit_t *RandomTrits, int length);
int init_suites(void) {return 0;}
int clean_suites(void) {return 0;}

/************** Test case functions ****************/


void test_pearl_diver_search(void)
{
	PearlDiver pearl_diver;

	int nonce_size = 1;
	trit_t trits[TRANSACTION_LENGTH];// = { trit_1 };
	getRandomTrits(trits, TRANSACTION_LENGTH);

	clock_t start = clock(), diff;
	search(&pearl_diver, trits, TRANSACTION_LENGTH, nonce_size, 32);
	diff = clock() - start;
	
	CU_ASSERT(pearl_diver.nonceFound);
	
}

void *dosearch(void *ctx) {
	PearlDiver *pearl_diver = (PearlDiver *)ctx;

	int nonce_size = 18;
	trit_t trits[TRANSACTION_LENGTH];// = { trit_1 };
	getRandomTrits(trits, TRANSACTION_LENGTH);

	search(pearl_diver, trits, TRANSACTION_LENGTH, nonce_size, 32);
	return 0;
}
void test_pearl_diver_interrupt(void)
{
	PearlDiver pearl_diver;

	pthread_t tid;
	pthread_create(&tid,NULL,&dosearch,(void *)&pearl_diver);

	sleep(1);

	interrupt(&pearl_diver);

	sleep(1);
	
	CU_ASSERT(!pearl_diver.nonceFound);
	CU_ASSERT(pearl_diver.interrupted);
}

void getRandomTrits (trit_t *RandomTrits, int length) {
	int i = 0;
	srand(time(NULL));
	while (i < length) {
		RandomTrits[i] = rand() % 3 - 1;
		i++;
	}

}

static CU_TestInfo tests[] = {
	{"PearlDiver Search Test", test_pearl_diver_search},
	{"PearlDiver Interrupt Test",test_pearl_diver_interrupt},
	CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
	{ "suitename1", init_suites, clean_suites, NULL, NULL, tests },
	CU_SUITE_INFO_NULL,
};

ADD_SUITE(suites);

#include "../src/lib/pearcldiver.h"
#include "../src/lib/PearlDiver.h"

#include "cunit_include.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
typedef long trit_t;
/*
   static const trit_t iotacurl_tryte2trits_tbl[27][3] = {
   { 0,  0,  0}, { 1,  0,  0}, {-1,  1,  0},
   { 0,  1,  0}, { 1,  1,  0}, {-1, -1,  1},
   { 0, -1,  1}, { 1, -1,  1}, {-1,  0,  1},
   { 0,  0,  1}, { 1,  0,  1}, {-1,  1,  1},
   { 0,  1,  1}, { 1,  1,  1}, {-1, -1, -1},
   { 0, -1, -1}, { 1, -1, -1}, {-1,  0, -1},
   { 0,  0, -1}, { 1,  0, -1}, {-1,  1, -1},
   { 0,  1, -1}, { 1,  1, -1}, {-1, -1,  0},
   { 0, -1,  0}, { 1, -1,  0}, {-1,  0,  0},
   };

   void trytes2trits(trit_t *trits, const char *trytes, const size_t len) {
   for(size_t i=0; i<len; i++) {
   size_t idx = (trytes[i]=='9' ? 0 : trytes[i]-'A'+1);
   trits[3*i+0] = iotacurl_tryte2trits_tbl[idx][0];
   trits[3*i+1] = iotacurl_tryte2trits_tbl[idx][1];
   trits[3*i+2] = iotacurl_tryte2trits_tbl[idx][2];
   }
   }
   */

static int init_suite(void) {
	//pdcl = malloc(sizeof(PearCLDiver));
	return 0;
}
static int clean_suite(void) {
	//free(pdcl);
	return 0;
}
static void init_cl_test(void) {
}
static void teardown_cl_test(void) {
	//memset(pdcl,0,sizeof(PearCLDiver));
}

static void test_init_kernel(void) {
	PearCLDiver pdcl;
	init_pearcl(&pdcl);
}

static void test_search(void) {
	int mwm; 
	double time_per_cycle, t1,t2, t3;
	clock_t begin, one, two,end;
	PearCLDiver pdcl;
	init_pearcl(&pdcl);
	trit_t trits[TRANSACTION_LENGTH];
	memset(trits,0,sizeof(trit_t)*TRANSACTION_LENGTH);
	//trits = malloc(sizeof(TRANSACTION_LENGTH));

	//memcpy(trits,rand_trits,TRANSACTION_LENGTH);
	for(mwm=1; mwm<2; mwm++) {
		//for(i = 0; i < TRANSACTION_LENGTH; i++) trits[i] = rand_trits[i];
		pearcl_search(&pdcl, trits, TRANSACTION_LENGTH, mwm);
		CU_ASSERT(pdcl.pd.nonceFound);
		//CU_ASSERT(memcmp(trits, rand_trits, sizeof(trit_t)*TRANSACTION_LENGTH) == 0);
	}
	begin = clock();
	one = clock();
	two = clock();
	end = clock();
	time_per_cycle = (double)(end - begin) / CLOCKS_PER_SEC;
	t1 =  (double)(one - begin) / CLOCKS_PER_SEC;
	t2 =  (double)(two - one) / CLOCKS_PER_SEC;
	t3 =  (double)(end - one) / CLOCKS_PER_SEC;
}

static CU_TestInfo tests[] = {
	{"Test Init CL Kernels", test_init_kernel},
	{"Test Search ", test_search},
	CU_TEST_INFO_NULL,
};

static CU_SuiteInfo suites[] = {
	{ "CLContext Test Suite", init_suite, clean_suite, init_cl_test,teardown_cl_test, tests },
	CU_SUITE_INFO_NULL,
};

//ADD_SUITE(suites);

int main() {
	return run_tests(suites);
}

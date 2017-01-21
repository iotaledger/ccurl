#include "PearlDiver.h"
#include "pearcldiver.h"
#include "Curl.h"
#include "util/converter.h"
#include <stdlib.h>
#include <stdio.h>

//PearlDiver pearl_diver;
PearCLDiver pdcl;
int initialized = 0;
int cl_available = 0;

EXPORT int ccurl_pow_init() {
	if(!initialized) {
		size_t lc = pdcl.loop_count;
		memset(&pdcl, 0, sizeof(PearCLDiver));
		//memset(&pearl_diver, 0, sizeof(PearlDiver));
		pdcl.loop_count  = lc;
		cl_available = init_pearcl(&pdcl);
		initialized = 1;
	}
	return cl_available;
}
EXPORT void ccurl_pow_set_loop_count(size_t c) {
	if(c > 0) 
		pdcl.loop_count = c;
}

EXPORT void ccurl_pow_finalize() {
	initialized = 0;
	finalize_cl(&pdcl.cl);
}

EXPORT void ccurl_pow_interrupt() {
	interrupt(&pdcl.pd);
}

EXPORT char *ccurl_pow(char *trytes, int minWeightMagnitude) {
	init_converter();
	char *buf = NULL; //= malloc(sizeof(char)*TRYTE_LENGTH);
	size_t len = strlen(trytes);
	trit_t *trits = trits_from_trytes(trytes, len);

#ifdef DEBUG
	fprintf(stderr, "Welcome to CCURL, home of the ccurl. can I take your vector?\n");
#endif
	if(ccurl_pow_init() == 0) {
		if(pdcl.loop_count < 1) {
			ccurl_pow_set_loop_count(32);
		}
#ifdef DEBUG
		fprintf(stderr, "OpenCL Hashing with %lu loops...", pdcl.loop_count);
#endif
		pearcl_search(&pdcl, trits, len * 3, minWeightMagnitude);
	} 
	if(pdcl.pd.status != PD_FOUND && pdcl.pd.status != PD_INVALID && pdcl.pd.status != PD_INTERRUPTED) {
#ifdef DEBUG
		fprintf(stderr, "Thread Hashing...");
#endif
		pd_search(&(pdcl.pd), trits, len * 3, minWeightMagnitude, -1);
	}
	if(pdcl.pd.status == PD_FOUND) {
#ifdef DEBUG
		fprintf(stderr, "Pow Finished.\n");
#endif
		buf = trytes_from_trits(trits, 0, TRANSACTION_LENGTH);
	}

	//buf[TRYTE_LENGTH] = 0;
	free(trits);
	pdcl.pd.status = PD_FINISHED;
	return buf;
}

EXPORT char *ccurl_digest_transaction(char *trytes) {
	char *hash;
	init_converter();
	Curl curl;
	init_curl(&curl);
	size_t length = strlen(trytes);
	trit_t digest[HASH_LENGTH];
	trit_t *input = trits_from_trytes(trytes, length < TRYTE_LENGTH? length: TRYTE_LENGTH);
	absorb(&curl, input, 0, length*3);
	squeeze(&curl, digest, 0, HASH_LENGTH);
	hash = trytes_from_trits(digest, 0, HASH_LENGTH);
	//hash[HASH_LENGTH] = 0;
	return hash;
}

#include "PearlDiver.h"
#include "pearcldiver.h"
#include "Curl.h"
#include "util/converter.h"
#include <stdlib.h>
#include <stdio.h>

#ifndef DEBUG
#define DEBUG
#endif
PearlDiver pearl_diver;
PearCLDiver pdcl;
int initialized = 0;
int cl_available = 0;

int ccurl_pow_init() {
	if(!initialized) {
		memset(&pdcl, 0, sizeof(PearCLDiver));
		memset(&pearl_diver, 0, sizeof(PearlDiver));
		cl_available = init_pearcl(&pdcl);
		initialized = 1;
	}
	return cl_available;
}
void ccurl_pow_finalize() {
	finalize_cl(&pdcl.cl);
}

EXPORT char *ccurl_pow(char *trytes, int minWeightMagnitude) {
	init_converter();
	char *buf; //= malloc(sizeof(char)*TRYTE_LENGTH);
	trit_t *trits = trits_from_trytes(trytes, TRYTE_LENGTH);

#ifdef DEBUG
	fprintf(stderr, "Welcome to CCURL, home of the ccurl. can I take your vector?\n");
#endif
	if(ccurl_pow_init() == 0) {
#ifdef DEBUG
		fprintf(stderr, "OpenCL Hashing...");
#endif
		if(pearcl_search(&pdcl, trits, TRANSACTION_LENGTH, minWeightMagnitude)) {
#ifdef DEBUG
			fprintf(stderr, "Thread Hashing Fallback...");
#endif
			pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, minWeightMagnitude, -1);
		}
	} else {
#ifdef DEBUG
		fprintf(stderr, "Thread Hashing...");
#endif
		pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, minWeightMagnitude, -1);
	}

	buf = trytes_from_trits(trits, 0, TRANSACTION_LENGTH);
	//buf[TRYTE_LENGTH] = 0;
	free(trits);
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

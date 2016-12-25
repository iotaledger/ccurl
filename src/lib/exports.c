#include "PearlDiver.h"
#include "pearcldiver.h"
#include "Curl.h"
#include "util/converter.h"
#include <stdlib.h>
#include <stdio.h>

EXPORT char *ccurl_pow(char *trytes, int minWeightMagnitude) {
	init_converter();
	char *buf = malloc(sizeof(char)*TRYTE_LENGTH);
	trit_t *trits = trits_from_trytes(trytes, TRYTE_LENGTH);

	PearlDiver pearl_diver;
	PearCLDiver pdcl;
	if(init_pearcl(&pdcl) == 0) {
		fprintf(stderr, "OpenCL Hashing...");
		if(pearcl_search(&pdcl, trits, TRANSACTION_LENGTH, minWeightMagnitude)) {
			fprintf(stderr, "Thread Hashing...");
			pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, minWeightMagnitude, -1);
		}
	} else {
		fprintf(stderr, "Thread Hashing...");
		pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, minWeightMagnitude, -1);
	}

	buf = trytes_from_trits(trits, 0, TRANSACTION_LENGTH);
	buf[TRYTE_LENGTH] = 0;
	free(trits);
	return buf;
}

EXPORT char *ccurl_digest_transaction(char *trytes) {
	init_converter();
	Curl curl;
	size_t length = strlen(trytes);
	trit_t digest[HASH_LENGTH];

	absorb(&curl, trits_from_trytes(trytes, length), 0, length*3);
	squeeze(&curl, digest, 0, HASH_LENGTH);

	return trytes_from_trits(digest, 0, HASH_LENGTH);
}

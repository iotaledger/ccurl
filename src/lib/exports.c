#include "PearlDiver.h"
#include "Curl.h"
#include "util/converter.h"
#include <stdlib.h>

EXPORT char *ccurl_pow(char *trytes, int minWeightMagnitude) {
	init_converter();
	char *buf = malloc(sizeof(char)*TRYTE_LENGTH);
	trit_t *trits = trits_from_trytes(trytes, TRYTE_LENGTH);

	PearlDiver pearl_diver;
	pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, minWeightMagnitude, -1);

	buf = trytes_from_trits(trits, 0, TRANSACTION_LENGTH);
	buf[TRYTE_LENGTH] = 0;
	free(trits);
	return buf;
}

EXPORT char *ccurl_digest_transaction(char *trytes) {
	init_converter();
	Curl curl;
	trit_t digest[HASH_LENGTH];

	absorb(&curl, trits_from_trytes(trytes, TRYTE_LENGTH), 0, TRANSACTION_LENGTH);
	squeeze(&curl, digest, 0, HASH_LENGTH);

	return trytes_from_trits(digest, 0, HASH_LENGTH);
}

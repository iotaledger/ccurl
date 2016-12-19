#include "PearlDiver.h"
#include "util/converter.h"
#include <stdlib.h>

EXPORT char *ccurl_pow(char *trytes, int minWeightMagnitude) {
	init_converter();
	char *buf = malloc(sizeof(char)*TRYTE_LENGTH);
	trit_t *trits = trits_from_trytes(trytes, TRYTE_LENGTH);

	PearlDiver pearl_diver;
	pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, minWeightMagnitude, -1);

	buf = trytes_from_trits(trits, TRANSACTION_LENGTH);
	buf[TRYTE_LENGTH] = 0;
	free(trits);
	return buf;
}

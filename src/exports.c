#include "PearlDiver.h"
#include <stdlib.h>

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

void trits2trytes(char *trytes, const trit_t *trits, const size_t len) {
	for(size_t i=0; i<len; i+=3) {
		int j = trits[i];
		if(i+1 < len) {
			j += 3 * trits[i+1];
		}
		if(i+2 < len) {
			j += 9 * trits[i+2];
		}
		if(j < 0) {
			j += 27;
		}
		trytes[i/3] = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ"[j];
	}
}

EXPORT char *ccurl_pow(char *trytes, int minWeightMagnitude) {
	char *buf = malloc(sizeof(char)*TRYTE_LENGTH);
	trit_t trits[TRANSACTION_LENGTH];

	trytes2trits(trits, trytes, TRYTE_LENGTH);
	PearlDiver pearl_diver;
	pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, minWeightMagnitude, -1);
	trits2trytes(buf, trits, TRANSACTION_LENGTH);

	buf[TRYTE_LENGTH] = 0;
	return buf;
}

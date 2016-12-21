
#ifndef PEARLDIVER_H
#define PEARLDIVER_H


#include <pthread.h>
#include <stdbool.h>
#include "Hash.h"

#define Invalid_transaction_trits_length 0x63
#define Invalid_min_weight_magnitude 0x64
#define InterruptedException 0x65
#define HIGH_BITS 0b1111111111111111111111111111111111111111111111111111111111111111L
#define LOW_BITS 0b0000000000000000000000000000000000000000000000000000000000000000L

typedef struct {
	trit_t low[STATE_LENGTH];
	trit_t high[STATE_LENGTH];
} States;
typedef struct {
	volatile bool finished, interrupted, nonceFound;
	pthread_mutex_t new_thread_search;
	pthread_t *tid;
} PearlDiver;

EXPORT void init_pearldiver(PearlDiver *ctx);
EXPORT void interrupt(PearlDiver *ctx);
EXPORT bool pd_search(PearlDiver *ctx, trit_t *const transaction_trits, int length, const int min_weight_magnitude, int numberOfThreads);
EXPORT void pd_transform( trit_t *const stateLow, trit_t *const stateHigh, trit_t *const scratchpadLow, trit_t *const scratchpadHigh);
EXPORT void pd_increment(trit_t *const midStateCopyLow, trit_t *const midStateCopyHigh, const int fromIndex, const int toIndex);
EXPORT void pd_search_init(States *states, trit_t *transaction_trits);
EXPORT char *ccurl_pow(char *trytes, int min_weight_magnitude);
EXPORT char *ccurl_digest_transaction(char *trytes);
/*
void trytes2trits(trit_t *trits, const char *trytes, const size_t len);
void trits2trytes(char *trytes, const trit_t *trits, const size_t len);
*/

#endif

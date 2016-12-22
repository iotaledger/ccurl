
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
	trit_t mid_low[STATE_LENGTH];
	trit_t mid_high[STATE_LENGTH];
	trit_t low[STATE_LENGTH];
	trit_t high[STATE_LENGTH];
} States;
typedef struct {
	volatile bool finished, interrupted, nonceFound;
	pthread_mutex_t new_thread_search;
	pthread_t *tid;
} PearlDiver;

void init_pearldiver(PearlDiver *ctx);
void interrupt(PearlDiver *ctx);
bool pd_search(PearlDiver *ctx, trit_t *const transaction_trits, int length, const int min_weight_magnitude, int numberOfThreads);
void pd_transform( trit_t *const stateLow, trit_t *const stateHigh, trit_t *const scratchpadLow, trit_t *const scratchpadHigh);
void pd_increment(trit_t *const midStateCopyLow, trit_t *const midStateCopyHigh, const int fromIndex, const int toIndex);
void pd_search_init(States *states, trit_t *transaction_trits);

#endif

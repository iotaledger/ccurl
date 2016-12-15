
#ifndef PEARLDIVER_H
#define PEARLDIVER_H

#include <pthread.h>
#include <stdbool.h>
#include "Hash.h"

#define Invalid_transaction_trits_length 0x63
#define Invalid_min_weight_magnitude 0x64
#define InterruptedException 0x65
#define TRANSACTION_LENGTH 8019
#define HIGH_BITS 0b1111111111111111111111111111111111111111111111111111111111111111L
#define LOW_BITS 0b0000000000000000000000000000000000000000000000000000000000000000L

typedef struct {
	volatile bool finished, interrupted, nonceFound;
	pthread_mutex_t new_thread_search;
	pthread_t *tid;
} PearlDiver;

void init_pearldiver(PearlDiver *ctx);
void interrupt(PearlDiver *ctx);
bool search(PearlDiver *ctx, trit_t *const transactionTrits, int length, const int minWeightMagnitude, int numberOfThreads);
void pd_transform( long *const stateLow, long *const stateHigh, long *const scratchpadLow, long *const scratchpadHigh);
void pd_increment(long *const midStateCopyLow, long *const midStateCopyHigh, const int fromIndex, const int toIndex);

#endif

#include "Hash.h"
#include "PearlDiver.h"
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>

typedef struct States {
	long *low;
	long *high;
	long *trits;
	int minWeightMagnitude;
	int threadIndex;
	PearlDiver *ctx;
} States;


void *find_nonce(void *states);
static inline void transform( long *const stateLow, long *const stateHigh, long *const scratchpadLow, long *const scratchpadHigh);
static void increment(long *const midStateCopyLow, long *const midStateCopyHigh, const int fromIndex, const int toIndex);

void interrupt(PearlDiver *ctx) {

	pthread_mutex_lock(&ctx->new_thread_search);
	ctx->finished = true;
	ctx->interrupted = true;

	pthread_mutex_unlock(&ctx->new_thread_search);
}

bool search(PearlDiver *ctx, long *const transactionTrits, int length, const int minWeightMagnitude, int numberOfThreads) {

	int i, j,k, thread_count;
	int offset = 0;
	long midStateLow[STATE_LENGTH], midStateHigh[STATE_LENGTH];

	if (length != TRANSACTION_LENGTH) {
		return Invalid_transaction_trits_length;
	}
	if (minWeightMagnitude < 0 || minWeightMagnitude > HASH_LENGTH) {
		return Invalid_min_weight_magnitude;
	}

	ctx->finished = false;
	ctx->interrupted = false;
	ctx->nonceFound = false;

	{
		for (i = HASH_LENGTH; i < STATE_LENGTH; i++) {

			midStateLow[i] = HIGH_BITS;
			midStateHigh[i] = HIGH_BITS;
		}

		long scratchpadLow[STATE_LENGTH],scratchpadHigh[STATE_LENGTH];

		for (i = (TRANSACTION_LENGTH - HASH_LENGTH) / HASH_LENGTH; i-- > 0; ) {

			for (j = 0; j < HASH_LENGTH; j++) {
				switch (transactionTrits[offset++]) {
					case 0: {
							midStateLow[j] = HIGH_BITS;
							midStateHigh[j] = HIGH_BITS;
						} break;
					case 1: {
							midStateLow[j] = LOW_BITS;
							midStateHigh[j] = HIGH_BITS;
						} break;
					default: {
							 midStateLow[j] = HIGH_BITS;
							 midStateHigh[j] = LOW_BITS;
						 }
				}
			}

			transform(midStateLow, midStateHigh, scratchpadLow, scratchpadHigh);
		}
		midStateLow[0] = 0b1101101101101101101101101101101101101101101101101101101101101101L;
		midStateHigh[0] = 0b1011011011011011011011011011011011011011011011011011011011011011L;
		midStateLow[1] = 0b1111000111111000111111000111111000111111000111111000111111000111L;
		midStateHigh[1] = 0b1000111111000111111000111111000111111000111111000111111000111111L;
		midStateLow[2] = 0b0111111111111111111000000000111111111111111111000000000111111111L;
		midStateHigh[2] = 0b1111111111000000000111111111111111111000000000111111111111111111L;
		midStateLow[3] = 0b1111111111000000000000000000000000000111111111111111111111111111L;
		midStateHigh[3] = 0b0000000000111111111111111111111111111111111111111111111111111111L;
	}

	if (numberOfThreads <= 0) {
		numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN) - 1;
		if (numberOfThreads < 1)
			numberOfThreads = 1;
	}

	pthread_mutex_init(&ctx->new_thread_search, NULL);
	//pthread_cond_init(&cond_search, NULL);
	if (pthread_mutex_lock(&ctx->new_thread_search) != 0) {
		printf("mutex init failed");
		return 1;
	}
	pthread_mutex_unlock(&ctx->new_thread_search);

	pthread_t tid[numberOfThreads];
	thread_count = numberOfThreads;

	while (numberOfThreads-- > 0) {

		States states = {
			.low = midStateLow,
			.high = midStateHigh,
			.trits = transactionTrits,
			.minWeightMagnitude = minWeightMagnitude,
			.threadIndex = numberOfThreads,
			.ctx = ctx
		};
		pthread_create(&tid[numberOfThreads],NULL,&find_nonce,(void *)&states);
	}

	sched_yield();
	
	for(k = thread_count; k > 0; k--) 
		pthread_join(tid[k-1], NULL);

	return ctx->interrupted;
}

void *find_nonce(void *states){
	long midStateCopyLow[STATE_LENGTH],midStateCopyHigh[STATE_LENGTH];
	int i,bitIndex;
	States *my_states = (States *)states;

	PearlDiver *ctx = my_states->ctx;
	memcpy(midStateCopyLow, my_states->low, STATE_LENGTH*sizeof(long));
	memcpy(midStateCopyHigh, my_states->high, STATE_LENGTH*sizeof(long));


	for (i = my_states->threadIndex; i-- > 0; ) {
		increment(midStateCopyLow, midStateCopyHigh, HASH_LENGTH / 3, (HASH_LENGTH / 3) * 2);
	}

	long scratchpadLow[STATE_LENGTH],scratchpadHigh[STATE_LENGTH],stateLow[STATE_LENGTH],stateHigh[STATE_LENGTH];

	bool skip;
	while (!ctx->finished && !ctx->interrupted) {
		increment(midStateCopyLow, midStateCopyHigh, (HASH_LENGTH / 3) * 2, HASH_LENGTH);
		memcpy( stateLow, midStateCopyLow, STATE_LENGTH*sizeof(long));
		memcpy( stateHigh, midStateCopyHigh, STATE_LENGTH*sizeof(long));
		transform(stateLow, stateHigh, scratchpadLow, scratchpadHigh);


		for (bitIndex = 64; bitIndex-- > 0; ) {
			skip = false;
			 if(ctx->finished) {return 0;}
			for (i = my_states->minWeightMagnitude; i-- > 0; ) {
				if ((((long)(stateLow[HASH_LENGTH - 1 - i] >> bitIndex)) & 1) != (((long)(stateHigh[HASH_LENGTH - 1 - i] >> bitIndex)) & 1)) {
					skip = true;
					break;
				}
			}
			if(skip) continue;

			pthread_mutex_lock(&my_states->ctx->new_thread_search);
			if(ctx->finished) {
				pthread_mutex_unlock(&my_states->ctx->new_thread_search);
				return 0;
			}
			ctx->finished = true;
			for ( i = 0; i < HASH_LENGTH; i++) {
				my_states->trits[TRANSACTION_LENGTH - HASH_LENGTH + i] = ((((long)(midStateCopyLow[i] >> bitIndex)) & 1) == 0) ? 1 : (((((long)(midStateCopyHigh[i] >> bitIndex)) & 1) == 0) ? -1 : 0);
				my_states->ctx->nonceFound = true;
			}
			pthread_mutex_unlock(&my_states->ctx->new_thread_search);
			sched_yield();
			return 0;
		}
	}
	return 0;
}


static
inline void transform( long *const stateLow, long *const stateHigh, long *const scratchpadLow, long *const scratchpadHigh) {

	int scratchpadIndex = 0, round, stateIndex;
	long alpha, beta, gamma, delta;

	for (round = 27; round-- > 0; ) {
		memcpy( scratchpadLow, stateLow, STATE_LENGTH * sizeof(long));
		memcpy( scratchpadHigh, stateHigh, STATE_LENGTH* sizeof(long));

		for (stateIndex = 0; stateIndex < STATE_LENGTH; stateIndex++) {

			alpha = scratchpadLow[scratchpadIndex];
			beta = scratchpadHigh[scratchpadIndex];
			gamma = scratchpadHigh[scratchpadIndex += (scratchpadIndex < 365 ? 364 : -365)];
			delta = (alpha | (~gamma)) & (scratchpadLow[scratchpadIndex] ^ beta);

			stateLow[stateIndex] = ~delta;
			stateHigh[stateIndex] = (alpha ^ gamma) | delta;
		}
	}

}
static void increment(long *const midStateCopyLow, long *const midStateCopyHigh, const int fromIndex, const int toIndex) {

	int i;

	for (i = fromIndex; i < toIndex; i++) {

		if (midStateCopyLow[i] == LOW_BITS) {

			midStateCopyLow[i] = HIGH_BITS;
			midStateCopyHigh[i] = LOW_BITS;

		} else {

			if (midStateCopyHigh[i] == LOW_BITS) {

				midStateCopyHigh[i] = HIGH_BITS;

			} else {

				midStateCopyLow[i] = LOW_BITS;
			}

			break;
		}
	}
}

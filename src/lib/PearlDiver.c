#include "Hash.h"
#include "PearlDiver.h"
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>

typedef struct {
	States *states;
	trit_t *trits;
	int min_weight_magnitude;
	int threadIndex;
	PearlDiver *ctx;
} PDThread;


void *find_nonce(void *states);

void interrupt(PearlDiver *ctx) {

	pthread_mutex_lock(&ctx->new_thread_search);
	ctx->finished = true;
	ctx->interrupted = true;

	pthread_mutex_unlock(&ctx->new_thread_search);
}

bool pd_search(PearlDiver *ctx, trit_t *const transactionTrits, int length, const int min_weight_magnitude, int numberOfThreads) {

	int k, thread_count;

	if (length != TRANSACTION_LENGTH) {
		return Invalid_transaction_trits_length;
	}
	if (min_weight_magnitude < 0 || min_weight_magnitude > HASH_LENGTH) {
		return Invalid_min_weight_magnitude;
	}

	ctx->finished = false;
	ctx->interrupted = false;
	ctx->nonceFound = false;

	States *states;
	states = malloc(sizeof(States));
	pd_search_init(states, transactionTrits);

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

	PDThread pdthreads[numberOfThreads];
	while (numberOfThreads-- > 0) {

		pdthreads[numberOfThreads] = (PDThread){
			.states = states,
			.trits = transactionTrits + TRANSACTION_LENGTH - HASH_LENGTH,
			.min_weight_magnitude = min_weight_magnitude,
			.threadIndex = numberOfThreads,
			.ctx = ctx
		};
		pthread_create(&tid[numberOfThreads],NULL,&find_nonce,(void *)&(pdthreads[numberOfThreads]));
	}

	sched_yield();

	for(k = thread_count; k > 0; k--) 
		pthread_join(tid[k-1], NULL);

	return ctx->interrupted;
}

void pd_search_init(States *states, trit_t *transactionTrits) {
	int i, j, offset = 0;
	for (i = HASH_LENGTH; i < STATE_LENGTH; i++) {

		states->mid_low[i] = HIGH_BITS;
		states->mid_high[i] = HIGH_BITS;
	}

	trit_t scratchpadLow[STATE_LENGTH],scratchpadHigh[STATE_LENGTH];

	for (i = (TRANSACTION_LENGTH - HASH_LENGTH) / HASH_LENGTH; i-- > 0; ) {

		for (j = 0; j < HASH_LENGTH; j++) {
			switch (transactionTrits[offset++]) {
				case 0: {
							states->mid_low[j] = HIGH_BITS;
							states->mid_high[j] = HIGH_BITS;
						} break;
				case 1: {
							states->mid_low[j] = LOW_BITS;
							states->mid_high[j] = HIGH_BITS;
						} break;
				default: {
							 states->mid_low[j] = HIGH_BITS;
							 states->mid_high[j] = LOW_BITS;
						 }
			}
		}

		pd_transform(states->mid_low, states->mid_high, scratchpadLow, scratchpadHigh);
	}
	states->mid_low[0] = 0b1101101101101101101101101101101101101101101101101101101101101101L;
	states->mid_high[0] = 0b1011011011011011011011011011011011011011011011011011011011011011L;
	states->mid_low[1] = 0b1111000111111000111111000111111000111111000111111000111111000111L;
	states->mid_high[1] = 0b1000111111000111111000111111000111111000111111000111111000111111L;
	states->mid_low[2] = 0b0111111111111111111000000000111111111111111111000000000111111111L;
	states->mid_high[2] = 0b1111111111000000000111111111111111111000000000111111111111111111L;
	states->mid_low[3] = 0b1111111111000000000000000000000000000111111111111111111111111111L;
	states->mid_high[3] = 0b0000000000111111111111111111111111111111111111111111111111111111L;
}

int is_found(trit_t *low, trit_t *high, int index, int min_weight_magnitude) {
	int i;
	for (i = min_weight_magnitude; i-- > 0; ) {
		if ((((trit_t)(low[HASH_LENGTH - 1 - i] )) &  (1 << index)) 
				!= (((trit_t)(high[HASH_LENGTH - 1 - i] )) & (1<< index))) {
			return 0;
		}
	}
	return 1;
}

int is_found_fast(trit_t *low, trit_t *high, int min_weight_magnitude) {
	int i;
	trit_t lastMeasurement = HIGH_BITS; /* (low[index] >> bitIndex & 1) != (high[index] >> bitIndex & 1) */
	for (i = min_weight_magnitude; i-- > 0; ) {
		lastMeasurement &= ~(low[HASH_LENGTH - 1 - i] ^ high[HASH_LENGTH - 1 - i]);
		if(lastMeasurement == 0) {
			return 0;
		}
	}
	return lastMeasurement;
}

void *find_nonce(void *date){
	trit_t midStateCopyLow[STATE_LENGTH],midStateCopyHigh[STATE_LENGTH];
	int i,nonce_probe;
	PDThread *my_thread= (PDThread *)date;
	trit_t *trits = my_thread->trits;

	memset(midStateCopyLow, 0, STATE_LENGTH*sizeof(trit_t));
	memset(midStateCopyHigh, 0, STATE_LENGTH*sizeof(trit_t));
	PearlDiver *ctx = my_thread->ctx;
	memcpy(midStateCopyLow, my_thread->states->mid_low, STATE_LENGTH*sizeof(trit_t));
	memcpy(midStateCopyHigh, my_thread->states->mid_high, STATE_LENGTH*sizeof(trit_t));


	for (i = my_thread->threadIndex; i-- > 0; ) {
		pd_increment(midStateCopyLow, midStateCopyHigh, HASH_LENGTH / 3, (HASH_LENGTH / 3) * 2);
	}

	trit_t scratchpadLow[STATE_LENGTH],scratchpadHigh[STATE_LENGTH],stateLow[STATE_LENGTH],stateHigh[STATE_LENGTH];
	memset(stateLow, 0, STATE_LENGTH*sizeof(trit_t));
	memset(stateHigh, 0, STATE_LENGTH*sizeof(trit_t));
	memset(scratchpadLow, 0, STATE_LENGTH*sizeof(trit_t));
	memset(scratchpadHigh, 0, STATE_LENGTH*sizeof(trit_t));
	while (!ctx->finished && !ctx->interrupted) {
		pd_increment(midStateCopyLow, midStateCopyHigh, (HASH_LENGTH / 3) * 2, HASH_LENGTH);
		memcpy( stateLow, midStateCopyLow, STATE_LENGTH*sizeof(trit_t));
		memcpy( stateHigh, midStateCopyHigh, STATE_LENGTH*sizeof(trit_t));
		pd_transform(stateLow, stateHigh, scratchpadLow, scratchpadHigh);

		if((nonce_probe = is_found_fast(stateLow,	stateHigh, 
					my_thread->min_weight_magnitude)) == 0)
			continue;
		pthread_mutex_lock(&my_thread->ctx->new_thread_search);
		if(ctx->finished) {
			pthread_mutex_unlock(&my_thread->ctx->new_thread_search);
			return 0;
		}
		ctx->finished = true;
		for ( i = 0; i < HASH_LENGTH; i++) {
			trits[i] = 
				(((trit_t)(midStateCopyLow[i] ) & nonce_probe) == 0) ? 
				1 : ((((trit_t)(midStateCopyHigh[i] ) & nonce_probe) == 0) ? -1 : 0);
		}
		my_thread->ctx->nonceFound = true;
		pthread_mutex_unlock(&my_thread->ctx->new_thread_search);
		sched_yield();
		return 0;
						
	}
	return 0;
}

void pd_transform( trit_t *const stateLow, trit_t *const stateHigh, trit_t *const scratchpadLow, trit_t *const scratchpadHigh) {

	int scratchpadIndex = 0, round, stateIndex;
	trit_t alpha, beta, gamma, delta;

	for (round = 27; round-- > 0; ) {
		memcpy( scratchpadLow, stateLow, STATE_LENGTH * sizeof(trit_t));
		memcpy( scratchpadHigh, stateHigh, STATE_LENGTH* sizeof(trit_t));

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
void pd_increment(trit_t *const midStateCopyLow, trit_t *const midStateCopyHigh, const int fromIndex, const int toIndex) {

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
	/*
	   bool skip;
	   */
/*
   for (i = my_thread->min_weight_magnitude; i-- > 0; ) {
   if ((((trit_t)(stateLow[HASH_LENGTH - 1 - i] >> bitIndex)) & 1)
   != (((trit_t)(stateHigh[HASH_LENGTH - 1 - i] >> bitIndex)) & 1)) {
   goto NEXT_BIT_INDEX;
   if ((((trit_t)(stateLow[HASH_LENGTH - 1 - i] )) &  (1 << bitIndex)) 
   != (((trit_t)(stateHigh[HASH_LENGTH - 1 - i] )) & (1<< bitIndex))) {
   skip = true;
   break;
   }
   }
   */
/*
   my_thread->trits[TRANSACTION_LENGTH - HASH_LENGTH + i] = 
   ((((trit_t)(midStateCopyLow[i] >> bitIndex)) & 1) == 0) ? 
1 : (((((trit_t)(midStateCopyHigh[i] >> bitIndex)) & 1) == 0) ? -1 : 0);
*/
/*
NEXT_BIT_INDEX:
continue;
*/


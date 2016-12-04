#include "Hash.h"
#include "PearlDiver.h"
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
*/
struct _PearlDiver {
	volatile bool finished, interrupted, nonceFound;
	pthread_mutex_t new_thread_search;
	pthread_mutex_t new_thread_interrupt;
};

static inline void transform( long *const stateLow, long *const stateHigh, long *const scratchpadLow, long *const scratchpadHigh);
static void increment(long *const midStateCopyLow, long *const midStateCopyHigh, const int fromIndex, const int toIndex);
void getRandomTrits (int *RandomTrits, int length);

// PearlDiver public functions
void interrupt(PearlDiver pearl_diver);
bool search(PearlDiver pearl_diver, int *const transactionTrits, int length, const int minWeightMagnitude, int numberOfThreads);

void interrupt(PearlDiver pearl_diver) {

	pthread_mutex_lock(&pearl_diver->new_thread_interrupt);
	pearl_diver->finished = true;
	pearl_diver->interrupted = true;

	//notifyAll();
	pthread_mutex_unlock(&pearl_diver->new_thread_interrupt);
}

bool search(PearlDiver pearl_diver, int *const transactionTrits, int length, const int minWeightMagnitude, int numberOfThreads) {

	int i, j;
	int offset = 0;
	int bitIndex;
	int threadIndex;

	// long scratchpadLow[243*3]; << This Method of declaration puts the array on the stack. 
	// In this case, that is BAD. This belongs on the heap. Keep the stack clean. Use pointers and malloc
	long *scratchpadLow, *scratchpadHigh, 
	     *midStateCopyLow, *midStateCopyHigh,
	     *stateLow, *stateHigh,
	     *midStateLow, *midStateHigh;
	scratchpadLow = malloc(STATE_LENGTH*sizeof(long));
	scratchpadHigh = malloc(STATE_LENGTH*sizeof(long));
	midStateCopyLow = malloc(STATE_LENGTH*sizeof(long));
	midStateCopyHigh = malloc(STATE_LENGTH*sizeof(long));
	stateLow = malloc(STATE_LENGTH*sizeof(long));
	stateHigh = malloc(STATE_LENGTH*sizeof(long));
	midStateLow = malloc(STATE_LENGTH*sizeof(long));
	midStateHigh = malloc(STATE_LENGTH*sizeof(long));

	if (length != TRANSACTION_LENGTH) {

		return Invalid_transaction_trits_length;
	}
	if (minWeightMagnitude < 0 || minWeightMagnitude > HASH_LENGTH) {

		return Invalid_min_weight_magnitude;
	}
	
	pearl_diver->finished = false;
	pearl_diver->interrupted = false;
	pearl_diver->nonceFound = false;

	{
		for (i = HASH_LENGTH; i < STATE_LENGTH; i++) {

			midStateLow[i] = HIGH_BITS;
			midStateHigh[i] = HIGH_BITS;
		}


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
		if (numberOfThreads < 1) {
			numberOfThreads = 1;
		}
	}

	while (numberOfThreads-- > 0) {
		threadIndex = numberOfThreads;
		pthread_mutex_lock(&pearl_diver->new_thread_search);
		memcpy(midStateCopyLow, midStateLow, STATE_LENGTH*sizeof(long));
		memcpy(midStateCopyHigh, midStateHigh, STATE_LENGTH*sizeof(long));
		for (i = threadIndex; i-- > 0; ) {

			increment(midStateCopyLow, midStateCopyHigh, HASH_LENGTH / 3, (HASH_LENGTH / 3) * 2);
		}

		while (!pearl_diver->finished) {

			increment(midStateCopyLow, midStateCopyHigh, (HASH_LENGTH / 3) * 2, HASH_LENGTH);
			memcpy( stateLow, midStateCopyLow, STATE_LENGTH*sizeof(long));
			memcpy( stateHigh, midStateCopyHigh, STATE_LENGTH*sizeof(long));
			transform(stateLow, stateHigh, scratchpadLow, scratchpadHigh);

		//NEXT_BIT_INDEX:
			for (bitIndex = 64; bitIndex-- > 0; ) {

				for (i = minWeightMagnitude; i-- > 0; ) {

					if ((((int)(stateLow[HASH_LENGTH - 1 - i] >> bitIndex)) & 1) != (((int)(stateHigh[HASH_LENGTH - 1 - i] >> bitIndex)) & 1)) {

						goto NEXT_BIT_INDEX;
					}
				}

				pearl_diver->finished = true;
				for ( i = 0; i < HASH_LENGTH; i++) {

					transactionTrits[TRANSACTION_LENGTH - HASH_LENGTH + i] = ((((int)(midStateCopyLow[i] >> bitIndex)) & 1) == 0) ? 1 : (((((int)(midStateCopyHigh[i] >> bitIndex)) & 1) == 0) ? -1 : 0);
					pearl_diver->nonceFound = true;
				}
				break;
				NEXT_BIT_INDEX:
				;
			}
		}
		pthread_mutex_unlock(&pearl_diver->new_thread_search);
	}

	if (wait(NULL))
		return InterruptedException;

	return pearl_diver->interrupted;
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


void getRandomTrits (int *RandomTrits, int length) {

	int i = 0;

	//int RandomTrits[length]; It's fine to declare these like they are java arrays; normally, you would put it on the heap, though, with a call to malloc().

	srand(time(NULL));

	while (i < length) {

		RandomTrits[i] = rand() % 3 - 1;

		/* Original code in comment:
		char RandomTrit = rand() % 3 - 1;
		if (RandomTrit == 0)
			RandomTrits[i] = -1;
		if (RandomTrit == 1)
			RandomTrits[i] = 0;
		if (RandomTrit == 2)
			RandomTrits[i] = 1;
		*/
		i++;
	}

	/* not necessary for debugging. you already have your randomtrits outside here. just do this shit in your test code
	for(i = 0; i < length; i++)
		printf("%d", RandomTrits[i]);
	*/

	//return RandomTrits;
}

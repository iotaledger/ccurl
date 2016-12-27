
#ifndef PEARLDIVER_H
#define PEARLDIVER_H

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <stdbool.h>
#include "Hash.h"

#define Invalid_transaction_trits_length 0x63
#define Invalid_min_weight_magnitude 0x64
#define InterruptedException 0x65
//#define HIGH_BITS 0b1111111111111111111111111111111111111111111111111111111111111111L
//#define LOW_BITS 0b0000000000000000000000000000000000000000000000000000000000000000L
#define HIGH_BITS 0xFFFFFFFFFFFFFFFF
#define LOW_BITS 0x0000000000000000
#define LOW_0 0xDB6DB6DB6DB6DB6D
#define HIGH_0 0xB6DB6DB6DB6DB6DB
#define LOW_1 0xF1F8FC7E3F1F8FC7
#define HIGH_1 0x8FC7E3F1F8FC7E3F
#define LOW_2 0x7FFFE00FFFFC01FF
#define HIGH_2 0xFFC01FFFF803FFFF
#define LOW_3 0xFFC0000007FFFFFF
#define HIGH_3 0x003FFFFFFFFFFFFF

typedef struct {
	trit_t mid_low[STATE_LENGTH];
	trit_t mid_high[STATE_LENGTH];
	trit_t low[STATE_LENGTH];
	trit_t high[STATE_LENGTH];
} States;
typedef struct {
	volatile bool finished, interrupted, nonceFound;
#ifdef _WIN32
	CRITICAL_SECTION new_thread_search;
#else
	pthread_mutex_t new_thread_search;
#endif
} PearlDiver;

void init_pearldiver(PearlDiver *ctx);
void interrupt(PearlDiver *ctx);
bool pd_search(PearlDiver *ctx, trit_t *const transaction_trits, int length, const int min_weight_magnitude, int numberOfThreads);
void pd_transform( trit_t *const stateLow, trit_t *const stateHigh, trit_t *const scratchpadLow, trit_t *const scratchpadHigh);
void pd_increment(trit_t *const midStateCopyLow, trit_t *const midStateCopyHigh, const int fromIndex, const int toIndex);
void pd_search_init(States *states, trit_t *transaction_trits);

#endif

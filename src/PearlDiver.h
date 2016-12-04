
#ifndef PEARLDIVER_H
#define PEARLDIVER_H

#include <pthread.h>
#include <stdbool.h>

#define Invalid_transaction_trits_length 0x63
#define Invalid_min_weight_magnitude 0x64
#define InterruptedException 0x65
#define TRANSACTION_LENGTH 8019
#define HIGH_BITS 0b1111111111111111111111111111111111111111111111111111111111111111L
#define LOW_BITS 0b0000000000000000000000000000000000000000000000000000000000000000L

struct _PearlDiver;
typedef struct _PearlDiver *PearlDiver;
void interrupt(PearlDiver pearl_diver);
bool search(PearlDiver pearl_diver, int *const transactionTrits, int length, const int minWeightMagnitude, int numberOfThreads);

/*
extern PearlDiver *pearl_diver__create(void);
extern void pearl_diver__create(PearlDiver *pearl_diver);
extern void pearl_diver__interrupt(void);
extern bool pearl_diver__search(int *const, int, const int, int);

static const struct {
	PearlDiver *(* create)(void);
	void (* interrupt)(void);
	bool (* search)(int *const, int, const int, int);
} pearl_diver = {
	pearl_diver__create,
	pearl_diver__interrupt,
	pearl_diver__search
};
*/

#endif

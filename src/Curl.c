/*
 * (c) 2016 Paul Handy, based on code from come-from-beyond
 */

#include "Hash.h"
#include "Curl.h"
#include "PearlDiver.h"
#include <stdlib.h>
#include <string.h>

#define __TRUTH_TABLE  1, 0, -1, 1, -1, 0, -1, 1, 0

struct Curl {
	int state[STATE_LENGTH];
};
static const int TRUTH_TABLE[9] = { __TRUTH_TABLE };

void absorb(Curl curl, int *const trits, int offset, int length);
void squeeze(Curl curl, int *const trits, int offset, int length);
void reset(Curl curl);
void transform(Curl curl);



void absorb(Curl curl, int *const trits, int offset, int length) {

	do {
		memcpy(curl->state, trits, (length < HASH_LENGTH? length: HASH_LENGTH) * sizeof(int));
		transform(curl);
		offset += HASH_LENGTH;
	} while ((length -= HASH_LENGTH) > 0);
}


void squeeze(Curl curl, int *const trits, int offset, int length) {

	do {
		memcpy(trits,  curl->state, (length < HASH_LENGTH? length: HASH_LENGTH) * sizeof(int));
		transform(curl);
		offset += HASH_LENGTH;
	} while ((length -= HASH_LENGTH) > 0);
}

void transform(Curl curl) {

	int *const scratchpad = malloc( sizeof(int) * STATE_LENGTH);
	int scratchpadIndex = 0;
	for (int round = 0; round < NUMBER_OF_ROUNDS; round++) {
		memcpy(scratchpad, curl->state,STATE_LENGTH * sizeof(int));
		for (int stateIndex = 0; stateIndex < STATE_LENGTH; stateIndex++) {
			curl->state[stateIndex] = TRUTH_TABLE[scratchpad[scratchpadIndex] + scratchpad[scratchpadIndex += (scratchpadIndex < 365 ? 364 : -365)] * 3 + 4];
		}
	}
}

void reset(Curl curl) {
	for (int stateIndex = 0; stateIndex < STATE_LENGTH; stateIndex++) {
		curl->state[stateIndex] = 0;
	}
}

/*
 * (c) 2016 Paul Handy, based on code from come-from-beyond
 */

#include "Hash.h"
#include "Curl.h"
#include "PearlDiver.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define __TRUTH_TABLE  	1, 0, -1, \
			1, -1, 0, \
			-1, 1, 0

static const long TRUTH_TABLE[9] = { __TRUTH_TABLE };

//static long state[STATE_LENGTH];
void transform(long *const state);

void init_curl(Curl *ctx) {
	ctx->state = malloc(sizeof(trit_t) * STATE_LENGTH);
}

void absorb(Curl *ctx, long *const trits, int offset, int length) {
	do {
		memcpy(ctx->state, &trits[offset], (length < HASH_LENGTH? length: HASH_LENGTH) * sizeof(long));
		transform(ctx->state);
		offset += HASH_LENGTH;
	} while ((length -= HASH_LENGTH) > 0);
}
void squeeze(Curl *ctx, long *const trits, int offset, int length) {
	do {
		memcpy(trits,  &ctx->state[offset], (length < HASH_LENGTH? length: HASH_LENGTH) * sizeof(long));
		transform(ctx->state);
		offset += HASH_LENGTH;
	} while ((length -= HASH_LENGTH) > 0);
}
/*
void absorb(long *const state, long *const trits, int offset, int length) {
	do {
		memcpy(state, &trits[offset], (length < HASH_LENGTH? length: HASH_LENGTH) * sizeof(long));
		transform(state);
		offset += HASH_LENGTH;
	} while ((length -= HASH_LENGTH) > 0);
}


void squeeze(long *const state, long *const trits, int offset, int length) {
	do {
		memcpy(trits,  &state[offset], (length < HASH_LENGTH? length: HASH_LENGTH) * sizeof(long));
		transform(state);
		offset += HASH_LENGTH;
	} while ((length -= HASH_LENGTH) > 0);
}
*/

void transform(long *const state ) {

	long *const scratchpad = malloc( sizeof(long) * STATE_LENGTH);
	int scratchpadIndex = 0;
	int scratchpadIndexSave;
	for (int round = 0; round < NUMBER_OF_ROUNDS; round++) {
		memcpy(scratchpad, state,STATE_LENGTH * sizeof(long));
		for (int stateIndex = 0; stateIndex < STATE_LENGTH; stateIndex++) {
			scratchpadIndexSave = scratchpadIndex;
			scratchpadIndex += (scratchpadIndex < 365 ? 364 : -365);
			state[stateIndex] = TRUTH_TABLE[scratchpad[scratchpadIndexSave ] + scratchpad[scratchpadIndex ] * 3 + 4];
		}
	}
}

void reset(Curl *ctx) {
	memset(ctx->state,0,STATE_LENGTH);
	/*
	for (int stateIndex = 0; stateIndex < STATE_LENGTH; stateIndex++) {
		state[stateIndex] = 0;
	}
	*/
}

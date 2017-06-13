/*
 * (c) 2016 Paul Handy, based on code from come-from-beyond
 */

#include "curl.h"
#include "hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __TRUTH_TABLE 1, 0, -1, 1, -1, 0, -1, 1, 0

static const trit_t TRUTH_TABLE[9] = {__TRUTH_TABLE};

void transform(Curl* ctx);

void init_curl(Curl* ctx) {
  // ctx->state = malloc(sizeof(trit_t) * STATE_LENGTH);
  memset(ctx->state, 0, STATE_LENGTH * sizeof(trit_t));
}
int i = 0;
void absorb(Curl* ctx, trit_t* const trits, int offset, int length) {
  do {
    memcpy(ctx->state, trits + offset,
           (length < HASH_LENGTH ? length : HASH_LENGTH) * sizeof(trit_t));
    transform(ctx);
    offset += HASH_LENGTH;
  } while ((length -= HASH_LENGTH) > 0);
}

void squeeze(Curl* ctx, trit_t* trits, int offset, int length) {
  do {
    // memcpy(trits+offset,  ctx->state, (length < HASH_LENGTH? length:
    // HASH_LENGTH) * sizeof(trit_t));
    memcpy(&(trits[offset]), ctx->state,
           (length < HASH_LENGTH ? length : HASH_LENGTH) * sizeof(trit_t));
    transform(ctx);
    offset += HASH_LENGTH;
  } while ((length -= HASH_LENGTH) > 0);
}

void transform(Curl* ctx) {
  trit_t scratchpad[STATE_LENGTH];
  int scratchpadIndex = 0;
  int scratchpadIndexSave;
  for (int round = 0; round < NUMBER_OF_ROUNDS; round++) {
    memcpy(scratchpad, ctx->state, STATE_LENGTH * sizeof(trit_t));
    for (int stateIndex = 0; stateIndex < STATE_LENGTH; stateIndex++) {
      scratchpadIndexSave = scratchpadIndex;
      scratchpadIndex += (scratchpadIndex < 365 ? 364 : -365);
      ctx->state[stateIndex] = TRUTH_TABLE[scratchpad[scratchpadIndexSave] +
                                           scratchpad[scratchpadIndex] * 3 + 4];
    }
  }
}

void reset(Curl* ctx) { memset(ctx->state, 0, STATE_LENGTH * sizeof(trit_t)); }

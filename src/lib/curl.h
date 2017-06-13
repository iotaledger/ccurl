
#ifndef CURL_H
#define CURL_H
#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUMBER_OF_ROUNDS 27

typedef struct _Curl { trit_t state[STATE_LENGTH]; } Curl;

EXPORT void init_curl(Curl* ctx);

EXPORT void absorb(Curl* ctx, trit_t* const trits, int offset, int length);
EXPORT void squeeze(Curl* ctx, trit_t* const trits, int offset, int length);
EXPORT void reset(Curl* ctx);

#endif

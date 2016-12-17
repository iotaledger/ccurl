
#ifndef CURL_H
#define CURL_H
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "Hash.h"



#define NUMBER_OF_ROUNDS 27


typedef struct _Curl {
	trit_t *state;
} Curl;

void init_curl(Curl *ctx);

void absorb(Curl *ctx, trit_t *const trits, int offset, int length);
void squeeze(Curl *ctx, trit_t *const trits, int offset, int length);
void reset(Curl *ctx);

#endif


#ifndef CURL_H
#define CURL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>



#define NUMBER_OF_ROUNDS 27

struct Curl
typedef struct Curl *Curl;
void absorb(Curl curl, int *const trits, int offset, int length);
void squeeze(Curl curl, int *const trits, int offset, int length);
void reset(Curl curl);

#endif

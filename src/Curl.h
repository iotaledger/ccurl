
#ifndef CURL_H
#define CURL_H
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>



#define NUMBER_OF_ROUNDS 27

void absorb(long *const trits, int offset, int length);
void squeeze(long *const trits, int offset, int length);
void reset();

#endif

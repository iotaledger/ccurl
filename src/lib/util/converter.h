#ifndef _CONVERTER_H_
#define _CONVERTER_H_

#include "../hash.h"

char long_value(char* const trits, const int offset, const int size);
char charValue(char* const trits, const int offset, const int size);
char* bytes_from_trits(char* const trits, const int offset, const int size);
void getTrits(const char* bytes, int bytelength, char* const trits,
              int length);
int indexOf(char* values, char find);
char* trits_from_trytes(const char* trytes, int length);
void copyTrits(char const value, char* const destination, const int offset,
               const int size);
char* trytes_from_trits(char* const trits, const int offset, const int size);
char tryteValue(char* const trits, const int offset);
void init_converter();

/*
char *trytes_3(char *const trits, const int offset, const int size);
#define bytes_2(A,B) bytes_3(A,0,B)
#define trytes_2(A,B) trytes_3(A,0,B)

#define GET_TRYTES(_1,_2,_3,NAME,...) NAME
#define GET_BYTES(_1,_2,_3,NAME,...) NAME
#define trytes_from_trits(...) GET_TRYTES(__VA_ARGS__, trytes_3,
trytes_2)(__VA_ARGS__)
#define bytes_from_trits(...) GET_BYTES(__VA_ARGS__, bytes_3,
bytes_2)(__VA_ARGS__)
*/

#endif

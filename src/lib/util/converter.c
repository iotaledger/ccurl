#include "converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define HASH_LENGTH 243
#define TRYTE_SPACE 27
#define MIN_TRYTE_VALUE -13
#define MAX_TRYTE_VALUE = 13
#define RADIX 3
#define MAX_TRIT_VALUE (RADIX - 1) / 2
#define MIN_TRIT_VALUE -MAX_TRIT_VALUE
#define NUMBER_OF_TRITS_IN_A_BYTE 5
#define NUMBER_OF_TRITS_IN_A_TRYTE 3
#define TRYTE_STRING "9ABCDEFGHIJKLMNOPQRSTUVWXYZ"

//#define mytrits[NUMBER_OF_TRITS_IN_A_BYTE]
// trit_t trits[NUMBER_OF_TRITS_IN_A_BYTE];
//#define IN_BYTE_TO_TRITS[HASH_LENGTH][]
//#define IN_TRYTE_TO_TRITS[HASH_LENGTH][]

/*
#define BLANK_BYTE_MAPPINGS
{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0},{0,0,0,0,0}
#define BLANK_TRYTE_MAPPINGS
{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}
*/

trit_t BYTE_TO_TRITS_MAPPINGS
    [HASH_LENGTH][NUMBER_OF_TRITS_IN_A_BYTE]; /*[HASH_LENGTH] = (trit_t *[]){
                                                 BLANK_BYTE_MAPPINGS };*/
trit_t TRYTE_TO_TRITS_MAPPINGS
    [TRYTE_SPACE][NUMBER_OF_TRITS_IN_A_TRYTE]; /*[TRYTE_SPACE] = (trit_t *[]){
                                                  BLANK_TRYTE_MAPPINGS };*/

static const char* TRYTE_ALPHABET = TRYTE_STRING;

trit_t long_value(trit_t* const trits, const int offset, const int size) {

  trit_t value = 0;
  for (int i = size; i-- > 0;) {
    value = value * RADIX + trits[offset + i];
  }
  return value;
}

char* bytes_from_trits(trit_t* const trits, const int offset, const int size) {
  int length =
      (size + NUMBER_OF_TRITS_IN_A_BYTE - 1) / NUMBER_OF_TRITS_IN_A_BYTE;
  char* bytes = (char*)malloc(sizeof(char) * length);
  for (int i = 0; i < length; i++) {

    trit_t value = 0;
    for (int j = (size - i * NUMBER_OF_TRITS_IN_A_BYTE) < 5
                     ? (size - i * NUMBER_OF_TRITS_IN_A_BYTE)
                     : NUMBER_OF_TRITS_IN_A_BYTE;
         j-- > 0;) {
      value = value * RADIX + trits[offset + i * NUMBER_OF_TRITS_IN_A_BYTE + j];
    }
    bytes[i] = (char)value;
  }

  return bytes;
}

void getTrits(const char* bytes, int bytelength, trit_t* const trits,
              int length) {

  int offset = 0;
  for (int i = 0; i < bytelength && offset < length; i++) {
    memcpy(
        trits + offset,
        BYTE_TO_TRITS_MAPPINGS
            [bytes[i] < 0
                 ? (bytes[i] +
                    HASH_LENGTH /* length of what? first? BYTE_TO_TRITS_MAPPINGS.length */)
                 : bytes[i]],
        sizeof(trit_t) * (length - offset < NUMBER_OF_TRITS_IN_A_BYTE
                              ? (length - offset)
                              : NUMBER_OF_TRITS_IN_A_BYTE));
    offset += NUMBER_OF_TRITS_IN_A_BYTE;
  }
  while (offset < length) {
    trits[offset++] = 0;
  }
}

trit_t* trits_from_trytes(const char* trytes, int length) {
  trit_t* trits = malloc(length * NUMBER_OF_TRITS_IN_A_TRYTE * sizeof(trit_t));
  for (int i = 0; i < length; i++) {
    memcpy(trits + i * NUMBER_OF_TRITS_IN_A_TRYTE,
           TRYTE_TO_TRITS_MAPPINGS[strchr(TRYTE_ALPHABET, trytes[i]) -
                                   TRYTE_ALPHABET],
           sizeof(trit_t) * NUMBER_OF_TRITS_IN_A_TRYTE);
  }

  return trits;
}

void copyTrits(trit_t const value, trit_t* const destination, const int offset,
               const int size) {

  trit_t absoluteValue = value < 0 ? -value : value;
  for (int i = 0; i < size; i++) {

    int remainder = (int)(absoluteValue % RADIX);
    absoluteValue /= RADIX;
    if (remainder > MAX_TRIT_VALUE) {

      remainder = MIN_TRIT_VALUE;
      absoluteValue++;
    }
    destination[offset + i] = remainder;
  }

  if (value < 0) {
    for (int i = 0; i < size; i++) {
      destination[offset + i] = -destination[offset + i];
    }
  }
}

char* trytes_from_trits(trit_t* const trits, const int offset, const int size) {

  const int length =
      (size + NUMBER_OF_TRITS_IN_A_TRYTE - 1) / NUMBER_OF_TRITS_IN_A_TRYTE;
  char* trytes = malloc(sizeof(char) * (length + 1));
  trytes[length] = '\0';
  for (int i = 0; i < length; i++) {
    trit_t j = trits[offset + i * 3] + trits[offset + i * 3 + 1] * 3 +
               trits[offset + i * 3 + 2] * 9;
    if (j < 0) {
      j += 27;
    }
    trytes[i] = TRYTE_ALPHABET[j];
  }
  return trytes;
}

trit_t tryteValue(trit_t* const trits, const int offset) {
  return trits[offset] + trits[offset + 1] * 3 + trits[offset + 2] * 9;
}

static void increment(trit_t* trits, int size) {
  int i;
  for (i = 0; i < size; i++) {
    if (++trits[i] > MAX_TRIT_VALUE) {
      trits[i] = MIN_TRIT_VALUE;
    } else {
      break;
    }
  }
}

void init_converter() {
  static char isInitialized = 0;

  if (isInitialized) {
    return;
  }

  trit_t trits[NUMBER_OF_TRITS_IN_A_BYTE];
  memset(trits, 0, NUMBER_OF_TRITS_IN_A_BYTE * sizeof(trit_t));
  for (int i = 0; i < HASH_LENGTH; i++) {
    memcpy(&(BYTE_TO_TRITS_MAPPINGS[i]), trits,
           NUMBER_OF_TRITS_IN_A_BYTE * sizeof(trit_t));
    // BYTE_TO_TRITS_MAPPINGS[i] = mytrits;/*Arrays.copyOf(trits,
    // NUMBER_OF_TRITS_IN_A_BYTE);*/
    increment(trits, NUMBER_OF_TRITS_IN_A_BYTE);
  }
  for (int i = 0; i < TRYTE_SPACE; i++) {
    memcpy(&(TRYTE_TO_TRITS_MAPPINGS[i]), trits,
           NUMBER_OF_TRITS_IN_A_TRYTE * sizeof(trit_t));
    // TRYTE_TO_TRITS_MAPPINGS[i] = mytrits; /*Arrays.copyOf(trits,
    // NUMBER_OF_TRITS_IN_A_TRYTE);*/
    increment(trits, NUMBER_OF_TRITS_IN_A_TRYTE);
  }

  isInitialized = 1;
}

/*
   static void increment(trit_t *const trits, const int size) {
   for (int i = 0; i < size; i++) {
   if (++trits[i] > Converter.MAX_TRIT_VALUE) {
   trits[i] = Converter.MIN_TRIT_VALUE;
   } else {
   break;
   }
   }
   }
   */

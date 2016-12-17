#ifndef HASH_H
#define HASH_H

#if defined(WIN32) || defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#include <stdint.h>
#define HASH_LENGTH 243
#define STATE_LENGTH 3 * HASH_LENGTH
#define TRYTE_LENGTH 2673
#define TRANSACTION_LENGTH TRYTE_LENGTH * 3
typedef int64_t trit_t;

#endif

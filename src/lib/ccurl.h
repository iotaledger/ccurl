#ifndef _CCURL_H_
#define _CCURL_H_

#if defined(WIN32) || defined(_WIN32)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#include <unistd.h>
#endif

EXPORT int ccurl_pow_init();
EXPORT void ccurl_pow_finalize();
EXPORT void ccurl_pow_set_loop_count(size_t c);
EXPORT char *ccurl_pow(char *trytes, int min_weight_magnitude);
EXPORT char *ccurl_digest_transaction(char *trytes);

#endif /* _CCURL_H_ */

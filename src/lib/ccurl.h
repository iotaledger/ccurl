#ifndef _CCURL_H_
#define _CCURL_H_

#include "util/converter.h"

EXPORT int ccurl_pow_init();
EXPORT void ccurl_pow_finalize();
EXPORT void ccurl_pow_set_loop_count(size_t c);
EXPORT char *ccurl_pow(char *trytes, int min_weight_magnitude);
EXPORT char *ccurl_digest_transaction(char *trytes);

#endif /* _CCURL_H_ */

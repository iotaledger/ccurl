#ifndef _CCURL_H_
#define _CCURL_H_

#include "util/converter.h"

EXPORT char *ccurl_pow(char *trytes, int min_weight_magnitude);
EXPORT char *ccurl_digest_transaction(char *trytes);

#endif /* _CCURL_H_ */

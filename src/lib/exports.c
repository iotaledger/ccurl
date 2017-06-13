#include "curl.h"
#include "util/converter.h"
#include <stdio.h>
#include <stdlib.h>

EXPORT char* ccurl_digest_transaction(char* trytes) {
  char* hash;
  init_converter();
  Curl curl;
  init_curl(&curl);
  size_t length = strlen(trytes);
  trit_t digest[HASH_LENGTH];
  trit_t* input =
      trits_from_trytes(trytes, length < TRYTE_LENGTH ? length : TRYTE_LENGTH);
  absorb(&curl, input, 0, length * 3);
  squeeze(&curl, digest, 0, HASH_LENGTH);
  hash = trytes_from_trits(digest, 0, HASH_LENGTH);
  // hash[HASH_LENGTH] = 0;
  free((void*) input);
  return hash;
}

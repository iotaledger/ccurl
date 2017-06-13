#include "curl.h"
#include "pearcldiver.h"
#include "pearl_diver.h"
#include "util/converter.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct pdcl_node {
  PearCLDiver* pdcl;
  int initialized;
  int cl_available;
  struct pdcl_node* next;
} pdcl_node_t;

pdcl_node_t base;
size_t loop_count = 32;
size_t offset = 0;

int ccurl_pow_node_init(pdcl_node_t* node) {
  if (!node->initialized) {
    node->pdcl = malloc(sizeof(PearCLDiver));
    // size_t lc = node->pdcl->loop_count;
    memset(node->pdcl, 0, sizeof(PearCLDiver));
    // memset(&pdcl, 0, sizeof(PearCLDiver));
    // memset(&pearl_diver, 0, sizeof(PearlDiver));
    // node->pdcl->loop_count = lc;
    node->cl_available = init_pearcl(node->pdcl);
    node->initialized = 1;
  }
  return node->cl_available;
}

EXPORT int ccurl_pow_init() {
  if (!base.initialized) {
    base.cl_available = ccurl_pow_node_init(&base);
  }
  return base.cl_available;
}

EXPORT void ccurl_pow_set_loop_count(size_t c) {
  if (c > 0)
    loop_count = c;
}

EXPORT void ccurl_pow_set_offset(size_t o) { offset = o; }

void ccurl_pow_node_finalize(pdcl_node_t* node) {
  if (node->next != NULL) {
    fputs("free next", stderr);
    ccurl_pow_node_finalize(node->next);
    free(node->next);
  }
  node->initialized = 0;
  finalize_cl(&(node->pdcl->cl));
  free(node->pdcl);
}

EXPORT void ccurl_pow_finalize() { ccurl_pow_node_finalize(&base); }

EXPORT void ccurl_pow_interrupt() {
  pdcl_node_t* node = &base;
  do {
    interrupt(&node->pdcl->pd);
    node = node->next;
  } while (node != NULL);
}

EXPORT char* ccurl_pow(char* trytes, int minWeightMagnitude) {
  init_converter();
  char* buf = NULL; //= malloc(sizeof(char)*TRYTE_LENGTH);
  size_t len = strlen(trytes);
  trit_t* trits = trits_from_trytes(trytes, len);
  pdcl_node_t* pd_node = &base;

#ifdef DEBUG
  fprintf(
      stderr,
      "Welcome to CCURL 0.2.1, home of the ccurl. can I take your vector?\n");
#endif
  ccurl_pow_node_init(pd_node);
  while (pd_node->pdcl->pd.status == PD_SEARCHING) {
    if (pd_node->next != NULL) {
      pd_node = pd_node->next;
    }
  }

  if (ccurl_pow_node_init(pd_node) == 0) {
    if (pd_node->pdcl->loop_count < 1) {
      pd_node->pdcl->loop_count = loop_count;
    }
#ifdef DEBUG
    fprintf(stderr, "OpenCL Hashing with %lu loops...",
            pd_node->pdcl->loop_count);
#endif
    pearcl_search(pd_node->pdcl, trits, offset, len * 3, minWeightMagnitude);
  }
  if (pd_node->pdcl->pd.status != PD_FOUND &&
      pd_node->pdcl->pd.status != PD_INVALID &&
      pd_node->pdcl->pd.status != PD_INTERRUPTED) {
#ifdef DEBUG
    fprintf(stderr, "Thread Hashing...");
#endif
    pd_search(&(pd_node->pdcl->pd), trits, len * 3, minWeightMagnitude, -1);
  }
  if (pd_node->pdcl->pd.status == PD_FOUND) {
#ifdef DEBUG
    fprintf(stderr, "Pow Finished.\n");
#endif
    buf = trytes_from_trits(trits, 0, TRANSACTION_LENGTH);
  }

  free(trits);
  pd_node->pdcl->pd.status = PD_FINISHED;
  return buf;
}

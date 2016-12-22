
#ifndef _PEARCLDIVER_H_
#define _PEARCLDIVER_H_
#include "claccess/clcontext.h"
#include "PearlDiver.h"
//#include "Hash.h"

typedef struct {
	CLContext cl;
	PearlDiver pd;
	size_t num_groups;
} PearCLDiver;

int init_pearcl(PearCLDiver *pd);
bool pearcl_search(PearCLDiver *pdcl, long *const trits, size_t length, size_t min_weight_magnitude);

#endif /* _PEARCLDIVER_H_ */

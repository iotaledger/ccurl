#include "pearcldiver.h"
#include "PearlDiver.h"
#include "claccess/clcontext.h"
#include "pearl.h.h"
#include "pearl.cl.h"
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

#define GROUP_SIZE 256

typedef struct {
	States *states;
	long *trits;
	size_t minWeightMagnitude;
	size_t index;
	PearCLDiver  *pdcl;
} PDCLThread;

void init_pearcl(PearCLDiver *pdcl) {
	unsigned char *src[] = (unsigned char*[]){ pearl_h, pearl_cl};
	size_t size[] = (size_t []){ pearl_h_len, pearl_cl_len };
	char *names[] = (char *[]){"init", "search"};//, "transform", "check", "finalize"};

	if(!pdcl) {
		pdcl = malloc(sizeof(PearCLDiver));
	}

	pdcl->cl.kernel.num_src = 2;
	pdcl->cl.kernel.num_kernels = 2;
	pdcl->cl.kernel.num_buffers = 7; 
	pdcl->cl.kernel.buffer[0] = (BufferInfo){sizeof(trit_t)*HASH_LENGTH, CL_MEM_WRITE_ONLY};   // trit hash //
	pdcl->cl.kernel.buffer[1] = (BufferInfo){sizeof(long), CL_MEM_READ_WRITE};                 // found //
	pdcl->cl.kernel.buffer[2] = (BufferInfo){sizeof(char), CL_MEM_READ_WRITE};                 // finished //
	pdcl->cl.kernel.buffer[3] = (BufferInfo){sizeof(trit_t)*STATE_LENGTH*4, CL_MEM_READ_WRITE, 2}; // states //
	pdcl->cl.kernel.buffer[4] = (BufferInfo){sizeof(size_t), CL_MEM_READ_ONLY};                // minweightmagnitude //
	pdcl->cl.kernel.buffer[5] = (BufferInfo){sizeof(long), CL_MEM_READ_WRITE, 1 };             // mask //
	pdcl->cl.kernel.buffer[6] = (BufferInfo){sizeof(size_t), CL_MEM_READ_WRITE, 1 };           // bitIndex //
	pd_init_cl(&(pdcl->cl), src, size, names);

	//pdcl->cl.kernel.buffer[5] = (BufferInfo){sizeof(size_t)*3*2, CL_MEM_READ_ONLY, 1};                // indices //
}

void *pearcl_find(void *data) {
	PDCLThread *thread = (PDCLThread *)data;
	PearCLDiver *pdcl = thread->pdcl;
	size_t local_work_size, global_work_size, global_offset;
	local_work_size = HASH_LENGTH; 
	global_work_size = local_work_size* 
		(pdcl->cl.num_cores[thread->index])* 
		pdcl->cl.num_multiple[thread->index];
	global_offset = local_work_size;
	unsigned long found = 0;
	char finished = 0;
	cl_event ev;
	trit_t states[STATE_LENGTH*2];

	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index], 
			pdcl->cl.buffers[thread->index][1], CL_TRUE, 0, sizeof(long), 
			&found, 0, NULL, NULL);
	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][2], CL_TRUE, 0, sizeof(char), 
			&finished, 0, NULL, NULL);
	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][3], CL_TRUE, 0, 
			STATE_LENGTH * sizeof(trit_t), thread->states->low, 0, NULL, NULL);
	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][3], CL_TRUE, 
			STATE_LENGTH * sizeof(trit_t), STATE_LENGTH * sizeof(trit_t),
			thread->states->high, 0, NULL, NULL);
	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][4], CL_TRUE, 0, 
			STATE_LENGTH * sizeof(trit_t), &(thread->minWeightMagnitude), 0, 
			NULL, NULL);

	check_clerror(clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index], 
				pdcl->cl.clkernel[thread->index][0], 1, &(thread->index), // &global_offset
				&global_work_size,&local_work_size, 0, NULL,&ev), 
			"E: running init kernel failed.\n");
	check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
				pdcl->cl.buffers[thread->index][2], CL_TRUE, 0, sizeof(char),
				&finished, 1, &ev, NULL), "E: could not read init errors.\n");
	if(finished != 0) {
		fprintf(stderr, "E: Index out of range.\n");
		exit(1);
	}
	int i = 0;
	do {
		i++;
		cl_event ev1;

		check_clerror(clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index], 
					pdcl->cl.clkernel[thread->index][1], 1, &(thread->index),
					&global_work_size,&local_work_size, 0, NULL, &ev1), 
				"E: running search kernel failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][2], CL_TRUE, 0, 
					sizeof(char), &finished, 1, &ev1, NULL),
				"E: reading finished bool failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][3], CL_TRUE, sizeof(trit_t)*STATE_LENGTH*6, sizeof(trit_t)*STATE_LENGTH*2,
					states, 0, NULL, NULL), "E: could not read states.\n");
		//fprintf(stderr, "\nFinished in %d tries: f%d \n",i, finished);
	} 
	while ( finished == 0 && !pdcl->pd.finished);
		//fprintf(stderr, "\nFinished in %d tries: f%d \n",i, finished);
		if(finished > 0) {
			/*
			check_clerror(clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index], 
						pdcl->cl.clkernel[thread->index][2],
						1, &(thread->index),&global_work_size,&local_work_size, 0, 
						NULL,&ev), "E: running finalize kernel failed.\n");
						*/
			pthread_mutex_lock(&pdcl->pd.new_thread_search);
			pdcl->pd.nonceFound = true;
			check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
						pdcl->cl.buffers[thread->index][0], CL_TRUE,
						0, 
						HASH_LENGTH* sizeof(trit_t), thread->trits, 1, &ev, NULL),
					"E: reading transaction hash failed.\n");
			//char trytes[HASH_LENGTH/3];
			//trits2trytes(trytes,thread->trits,HASH_LENGTH);
			//fprintf(stderr, "Fount hash: %s\n", trytes);
			pthread_mutex_unlock(&pdcl->pd.new_thread_search);
		}

		return 0;
}

bool pearcl_search(
		PearCLDiver *pdcl,
		long *const trits,
		size_t length,
		size_t minWeightMagnitude
		) {
	int k, thread_count;
	int numberOfThreads = pdcl->cl.num_devices;

	if (length != TRANSACTION_LENGTH) {
		return Invalid_transaction_trits_length;
	}
	if (minWeightMagnitude > HASH_LENGTH) {
		return Invalid_min_weight_magnitude;
	}
	trit_t hashTrits[HASH_LENGTH];

	pdcl->pd.finished = false;
	pdcl->pd.interrupted = false;
	pdcl->pd.nonceFound = false;

	States states;
	memset(&states,0,sizeof(States));
	pd_search_init(&states, trits);

	if (numberOfThreads <= 0) {
		numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN) - 1;
		if (numberOfThreads < 1)
			numberOfThreads = 1;
	}

	pthread_mutex_init(&pdcl->pd.new_thread_search, NULL);
	//pthread_cond_init(&cond_search, NULL);
	if (pthread_mutex_lock(&pdcl->pd.new_thread_search) != 0) {
		return 1;
	}
	pthread_mutex_unlock(&pdcl->pd.new_thread_search);

	pthread_t tid[numberOfThreads];
	thread_count = numberOfThreads;

	numberOfThreads = 1;
	while (numberOfThreads-- > 0) {
		PDCLThread pdthread = {
			.states = &states,
			.trits = hashTrits,
			.minWeightMagnitude = minWeightMagnitude,
			.index = numberOfThreads,
			.pdcl = pdcl
		};
		pthread_create(&tid[numberOfThreads], NULL, &pearcl_find, 
				(void *)&pdthread);
	}

	sched_yield();

	for(k = thread_count; k > 0; k--) {
		pthread_join(tid[k-1], NULL);
	}
	if(pdcl->pd.nonceFound) {
		memcpy(trits+TRANSACTION_LENGTH-HASH_LENGTH,hashTrits,HASH_LENGTH*sizeof(trit_t));
	}

	return pdcl->pd.interrupted;
}

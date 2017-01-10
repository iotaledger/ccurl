#include "pearcldiver.h"
#include "PearlDiver.h"
#include "Hash.h"
#include "claccess/clcontext.h"
#include "pearl.cl.h"
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#ifdef _WIN32
#else
#include <sched.h>
#endif

#ifndef PD_NUM_SRC
#define PD_NUM_SRC 1
#endif /*PD_NUM_SRC*/

typedef struct {
	States states;
	trit_t *trits;
	size_t min_weight_magnitude;
	size_t index;
	PearCLDiver  *pdcl;
} PDCLThread;

int init_pearcl(PearCLDiver *pdcl) {
	unsigned char *src[PD_NUM_SRC] = { pearl_cl };
	size_t size[PD_NUM_SRC] = { pearl_cl_len };
	char **names = (char *[]) { "init", "search", "finalize" };

	if (!pdcl) {
		pdcl = malloc(sizeof(PearCLDiver));
	}

	pdcl->cl.kernel.num_src = PD_NUM_SRC;
	pdcl->cl.kernel.num_kernels = 3;
	return pd_init_cl(&(pdcl->cl), src, size, names);
}

#ifdef _WIN32
DWORD WINAPI pearcl_find(void *data) {
#else
void *pearcl_find(void *data) {
#endif
	size_t local_work_size,
		global_work_size,
		global_offset = 0,
		num_groups;
	char found = 0;
	cl_event ev;
	cl_int errno;
	PDCLThread *thread;
	PearCLDiver *pdcl;
	thread = (PDCLThread *)data;
	pdcl = thread->pdcl;
	num_groups = (pdcl->cl.num_cores[thread->index]);// * pdcl->cl.num_multiple[thread->index];
	local_work_size = STATE_LENGTH;
	while (local_work_size > pdcl->cl.num_multiple[thread->index]) {
		local_work_size /= 3;
	}
	global_work_size = local_work_size * num_groups;

	for (int i = 0; i < thread->index; i++) {
		global_offset += pdcl->cl.num_cores[i];
	}
	if(CL_SUCCESS != 
		clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][1], CL_TRUE, 0,
			sizeof(trit_t)*STATE_LENGTH, &(thread->states.mid_low), 0, NULL, NULL)) {
		fprintf(stderr, "E: failed to write mid state low");
		return 0;
	}
	if(CL_SUCCESS != 
		clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][2], CL_TRUE, 0,
			sizeof(trit_t)*STATE_LENGTH, &(thread->states.mid_high), 0, NULL, NULL)) {
		fprintf(stderr, "E: failed to write mid state low");
		return 0;
	}
	if(CL_SUCCESS != 
		clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][5], CL_TRUE, 0,
			pdcl->cl.kernel.buffer[5].size, &(thread->min_weight_magnitude), 0,
			NULL, NULL)) {
		fprintf(stderr, "E: failed to write min_weight_magnitude");
		return 0;
	}
	if(CL_SUCCESS != 
		clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][8], CL_TRUE, 0,
			pdcl->cl.kernel.buffer[8].size, &(pdcl->loop_count), 0,
			NULL, NULL)) {
		fprintf(stderr, "E: failed to write min_weight_magnitude");
		return 0;
	}

	if(CL_SUCCESS != 
		(errno = clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.clkernel[thread->index][0], 1, &global_offset,
			&global_work_size, &local_work_size, 0, NULL, &ev))) {
		fprintf(stderr, "E: running init kernel failed with error %d.\n", errno);
		return 0;
	}
	if(CL_SUCCESS != 
		clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][6], CL_TRUE, 0, sizeof(char),
			&found, 1, &ev, NULL))
		fprintf(stderr, "E: could not read init errors.\n");
	while (found == 0 && !pdcl->pd.finished) {
		cl_event ev1;
	if(CL_SUCCESS != 
				clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.clkernel[thread->index][1], 1, NULL,
			&global_work_size, &local_work_size, 0, NULL, &ev1)) {
			fprintf(stderr, "E: running search kernel failed.\n");
			return 0;
	}
	if(CL_SUCCESS != 
			clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][6], CL_TRUE, 0,
			sizeof(char), &found, 1, &ev1, NULL))
			fprintf(stderr, "E: reading finished bool failed.\n");
	}
	if (found > 0) {
	if(CL_SUCCESS != 
				clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.clkernel[thread->index][2], 1, NULL,
			&global_work_size, &local_work_size, 0, NULL, &ev))
			fprintf(stderr, "E: running search kernel failed.\n");
#ifdef _WIN32
		EnterCriticalSection(&pdcl->pd.new_thread_search);
#else
		pthread_mutex_lock(&pdcl->pd.new_thread_search);
#endif
		if (pdcl->pd.nonceFound) return 0;
		pdcl->pd.nonceFound = true;
		pdcl->pd.finished = true;
		if(CL_SUCCESS != 
				clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][0], CL_TRUE,
			0, HASH_LENGTH * sizeof(trit_t), &(thread->trits[TRANSACTION_LENGTH - HASH_LENGTH]),
			1, &ev, NULL))
			fprintf(stderr, "E: reading transaction hash failed.\n");
#ifdef _WIN32
		LeaveCriticalSection(&pdcl->pd.new_thread_search);
#else
		pthread_mutex_unlock(&pdcl->pd.new_thread_search);
#endif
	}

	return 0;
}

bool pearcl_search(
	PearCLDiver *pdcl,
	trit_t *const trits,
	size_t length,
	size_t min_weight_magnitude
) {
	int k, thread_count;
	int numberOfThreads = pdcl->cl.num_devices;

	if (length != TRANSACTION_LENGTH) {
		return Invalid_transaction_trits_length;
	}
	if (min_weight_magnitude > HASH_LENGTH) {
		return Invalid_min_weight_magnitude;
	}

	pdcl->cl.kernel.num_buffers = 9;
	pdcl->cl.kernel.buffer[0] = (BufferInfo) { sizeof(trit_t)*HASH_LENGTH, CL_MEM_WRITE_ONLY };  // trit_hash //
	pdcl->cl.kernel.buffer[1] = (BufferInfo) { sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2 }; // states array  //
	pdcl->cl.kernel.buffer[2] = (BufferInfo) { sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2 }; // states array  //
	pdcl->cl.kernel.buffer[3] = (BufferInfo) { sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2 }; // states array  //
	pdcl->cl.kernel.buffer[4] = (BufferInfo) { sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2 }; // states array  //
	pdcl->cl.kernel.buffer[5] = (BufferInfo) { sizeof(size_t), CL_MEM_READ_ONLY };                		// minweightmagnitude //
	pdcl->cl.kernel.buffer[6] = (BufferInfo) { sizeof(char), CL_MEM_READ_WRITE };                 		// found //
	pdcl->cl.kernel.buffer[7] = (BufferInfo) { sizeof(trit_t), CL_MEM_READ_WRITE, 2 };           // nonce_probe //
	pdcl->cl.kernel.buffer[8] = (BufferInfo) { sizeof(size_t), CL_MEM_READ_ONLY};           // loop_length //

	if (kernel_init_buffers(&(pdcl->cl)) != 0) {
		//fprintf(stderr, "Could not init kernel buffers. \n");
		return true;
	}

	pdcl->pd.finished = false;
	pdcl->pd.interrupted = false;
	pdcl->pd.nonceFound = false;

	States states;
	pd_search_init(&states, trits);

	if (numberOfThreads == 0)
		return 1;


#ifdef _WIN32
	InitializeCriticalSection(&pdcl->pd.new_thread_search);
	HANDLE *tid = malloc(sizeof(HANDLE)*numberOfThreads);
#else
	pthread_mutex_init(&pdcl->pd.new_thread_search, NULL);
	if (pthread_mutex_lock(&pdcl->pd.new_thread_search) != 0) {
		return 1;
	}
	pthread_mutex_unlock(&pdcl->pd.new_thread_search);
	pthread_t *tid = malloc(numberOfThreads * sizeof(pthread_t));
#endif
	thread_count = numberOfThreads;

	PDCLThread  *pdthreads = (PDCLThread  *)malloc(numberOfThreads * sizeof(PDCLThread));
	while (numberOfThreads-- > 0) {
		pdthreads[numberOfThreads] = (PDCLThread) {
			.states = states,
				.trits = trits,
				.min_weight_magnitude = min_weight_magnitude,
				.index = numberOfThreads,
				.pdcl = pdcl
		};
#ifdef _WIN32
		tid[numberOfThreads] = CreateThread(NULL,0,&pearcl_find,(void *)&(pdthreads[numberOfThreads]),0,NULL);
#else
		pthread_create(&tid[numberOfThreads], NULL, &pearcl_find,
			(void *)&(pdthreads[numberOfThreads]));
#endif
	}

#ifdef _WIN32
	SwitchToThread();
	for (k = thread_count; k > 0; k--) {
		WaitForSingleObject(tid[k - 1], INFINITE);
	}
#else
	sched_yield();
	for (k = thread_count; k > 0; k--) {
		pthread_join(tid[k - 1], NULL);
	}
#endif

	free(tid);
	free(pdthreads);
	return pdcl->pd.interrupted;
}
/*
 *
	check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
				pdcl->cl.buffers[thread->index][1], CL_TRUE,
				0, sizeof(trit_t)*STATE_LENGTH, mid_low,
				0, NULL, NULL),
			"E: reading transaction hash failed.\n");
	assert(memcmp(thread->states.mid_low, mid_low, STATE_LENGTH*sizeof(trit_t)) == 0);
	*/

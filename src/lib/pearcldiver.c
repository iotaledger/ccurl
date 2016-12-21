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
	States states;
	long *trits;
	size_t min_weight_magnitude;
	size_t index;
	PearCLDiver  *pdcl;
} PDCLThread;

void init_pearcl(PearCLDiver *pdcl) {
	unsigned char *src[] = (unsigned char*[]){ pearl_h, pearl_cl};
	size_t size[] = (size_t []){ pearl_h_len, pearl_cl_len };
	char *names[] = (char *[]){"init", "search", "finalize"};

	if(!pdcl) {
		pdcl = malloc(sizeof(PearCLDiver));
	}

	pdcl->cl.kernel.num_src = 2;
	pdcl->cl.kernel.num_kernels = 3;
	pd_init_cl(&(pdcl->cl), src, size, names);
}

void *pearcl_find(void *data) {
	size_t local_work_size, 
		   global_work_size,
		   num_groups,
		   found_index,
		   i;
	char found = 0;
	cl_event ev;
	PDCLThread *thread;
	PearCLDiver *pdcl;
	thread = (PDCLThread *)data;
	pdcl = thread->pdcl;
	num_groups = 1;//(pdcl->cl.num_cores[thread->index]) * pdcl->cl.num_multiple[thread->index];
	local_work_size = HASH_LENGTH; 
	global_work_size = local_work_size * num_groups;
	char bit_indeces[num_groups];
	size_t write_difference;
	trit_t mid_state_low[STATE_LENGTH];
	trit_t mid_state_high[STATE_LENGTH];
	trit_t state_low[STATE_LENGTH];
	trit_t state_high[STATE_LENGTH];
	trit_t trit_hash[HASH_LENGTH];
	trit_t nonce_probe[num_groups];

	check_clerror(
	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][1], CL_TRUE, 0, 
			STATE_LENGTH * sizeof(trit_t), thread->states.low, 0, NULL, NULL),
	"E: failed to write mid state low");
	check_clerror(
	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][2], CL_TRUE, 0, 
			STATE_LENGTH * sizeof(trit_t), thread->states.high, 0, NULL, NULL),
	"E: failed to write mid state high");
	check_clerror(
	clEnqueueWriteBuffer(pdcl->cl.clcmdq[thread->index],
			pdcl->cl.buffers[thread->index][5], CL_TRUE, 0, 
			pdcl->cl.kernel.buffer[5].size, &(thread->min_weight_magnitude), 0, 
			NULL, NULL),
	"E: failed to write min_weight_magnitude");

	check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
				pdcl->cl.buffers[thread->index][2], CL_TRUE,
				0, HASH_LENGTH* sizeof(trit_t), mid_state_high,
				0, NULL, NULL),
			"E: reading transaction hash failed.\n");
	check_clerror(
			clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index], 
				pdcl->cl.clkernel[thread->index][0], 1, NULL,
				&global_work_size,&local_work_size, 0, NULL,&ev), 
			"E: running init kernel failed.\n");
	check_clerror(
			clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
				pdcl->cl.buffers[thread->index][6], CL_TRUE, 0, sizeof(char),
				&found, 1, &ev, NULL), "E: could not read init errors.\n");
	check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
				pdcl->cl.buffers[thread->index][2], CL_TRUE,
				0, STATE_LENGTH* sizeof(trit_t), mid_state_high,
				0, NULL, NULL),
			"E: reading transaction hash failed.\n");
	write_difference = memcmp(thread->states.high, mid_state_high, sizeof(trit_t)*STATE_LENGTH);
	int tries = 0;
	while( found == 0 && !pdcl->pd.finished) {
		tries++;
		cl_event ev1;
		check_clerror(clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index], 
					pdcl->cl.clkernel[thread->index][1], 1, NULL,
					&global_work_size,&local_work_size, 0, NULL, &ev1), 
				"E: running search kernel failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][6], CL_TRUE, 0, 
					sizeof(char), &found, 1, &ev1, NULL),
				"E: reading finished bool failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][0], CL_TRUE,
					0, HASH_LENGTH* sizeof(trit_t), trit_hash,
					0, NULL, NULL),
				"E: reading transaction hash failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][1], CL_TRUE,
					0, STATE_LENGTH* sizeof(trit_t), mid_state_low,
					0, NULL, NULL),
				"E: reading transaction hash failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][2], CL_TRUE,
					0, STATE_LENGTH* sizeof(trit_t), mid_state_high,
					0, NULL, NULL),
				"E: reading transaction hash failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][3], CL_TRUE,
					0, STATE_LENGTH* sizeof(trit_t), state_low,
					0, NULL, NULL),
				"E: reading transaction hash failed.\n");
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][4], CL_TRUE,
					0, STATE_LENGTH* sizeof(trit_t), state_high,
					0, NULL, NULL),
				"E: reading transaction hash failed.\n");
		pd_increment(thread->states.low, thread->states.high, (HASH_LENGTH/2)*3, HASH_LENGTH);
		write_difference = 
			memcmp(thread->states.high, mid_state_high, sizeof(trit_t)*STATE_LENGTH)
			+ memcmp(thread->states.low, mid_state_low, sizeof(trit_t)*STATE_LENGTH);
		if(tries == 20126 || found != 0) {
			check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
						pdcl->cl.buffers[thread->index][7], CL_TRUE,
						0, num_groups * sizeof(trit_t), nonce_probe,
						0, NULL, NULL),
					"E: reading transaction hash failed.\n");
		}
	} 
	if(found > 0) {
		check_clerror(clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index], 
					pdcl->cl.clkernel[thread->index][2], 1, NULL,
					&local_work_size,&local_work_size, 0, NULL, &ev), 
				"E: running search kernel failed.\n");
		pthread_mutex_lock(&pdcl->pd.new_thread_search);
		if(pdcl->pd.nonceFound) return 0;
		pdcl->pd.nonceFound = true;
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][0], CL_TRUE,
					0, HASH_LENGTH* sizeof(trit_t), trit_hash,
					1, &ev, NULL),
				"E: reading transaction hash failed.\n");
		memcpy(thread->trits + TRANSACTION_LENGTH - HASH_LENGTH, trit_hash, HASH_LENGTH*sizeof(trit_t));
		fprintf(stderr, "\nFound index: %d\n", (int)found);
		for(i = 0; i < HASH_LENGTH; i++) {
			fprintf(stderr, "%ld, ", trit_hash[i]);
		}
		fprintf(stderr, "\n");
		pthread_mutex_unlock(&pdcl->pd.new_thread_search);
	}

	return 0;
}

bool pearcl_search(
		PearCLDiver *pdcl,
		long *const trits,
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

	pdcl->cl.kernel.num_buffers = 8; 
	pdcl->cl.kernel.buffer[0] = (BufferInfo){sizeof(trit_t)*HASH_LENGTH, CL_MEM_WRITE_ONLY};  // trit_hash //
	pdcl->cl.kernel.buffer[1] = (BufferInfo){sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2}; // mid_state_low   //
	pdcl->cl.kernel.buffer[2] = (BufferInfo){sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2}; // mid_state_high  //
	pdcl->cl.kernel.buffer[3] = (BufferInfo){sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2}; // state_low //
	pdcl->cl.kernel.buffer[4] = (BufferInfo){sizeof(trit_t)*STATE_LENGTH, CL_MEM_READ_WRITE, 2}; // state_high//
	pdcl->cl.kernel.buffer[5] = (BufferInfo){sizeof(size_t), CL_MEM_READ_ONLY};                		// minweightmagnitude //
	pdcl->cl.kernel.buffer[6] = (BufferInfo){sizeof(char), CL_MEM_READ_WRITE};                 		// found //
	pdcl->cl.kernel.buffer[7] = (BufferInfo){sizeof(trit_t), CL_MEM_READ_WRITE, 2};           // nonce_probe //
	//pdcl->cl.kernel.buffer[8] = (BufferInfo){sizeof(size_t), CL_MEM_READ_WRITE, 1};                         // increment end//

	kernel_init_buffers (&(pdcl->cl));

	pdcl->pd.finished = false;
	pdcl->pd.interrupted = false;
	pdcl->pd.nonceFound = false;

	States states;
	//memset(&states,0,sizeof(States));
	pd_search_init(&states, trits);

	if (numberOfThreads == 0)
		return 1;

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
			.states = states,
			.trits = trits,
			.min_weight_magnitude = min_weight_magnitude,
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

	return pdcl->pd.interrupted;
}
		/*
		check_clerror(clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.buffers[thread->index][3], CL_TRUE,
					0, 
					sizeof(bit_indeces), bit_indeces, 1, &ev, NULL),
				"E: reading transaction hash failed.\n");
		for(i = 0; i < num_groups; i++) {
			if(bit_indeces[i] != -1) {
				found_index = i;
				break;
			}
		}
		*/
	/*
	trit_t hashTrits[HASH_LENGTH]; //from above
	if(pdcl->pd.nonceFound) {
		memcpy(trits+TRANSACTION_LENGTH-HASH_LENGTH,hashTrits,HASH_LENGTH*sizeof(trit_t));
	}
	*/

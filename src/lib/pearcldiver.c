#include "pearcldiver.h"
#include "pearl_diver.h"
#include "hash.h"
#include "claccess/clcontext.h"
#include "pearl.cl.h"
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#if defined(_WIN32) && !defined(__MINGW32__)
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
	if (pd_init_cl(&(pdcl->cl), src, size, names) != 0) {
		return 1;
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

	return kernel_init_buffers(&(pdcl->cl));
}

void pearcl_write_buffers(PDCLThread *thread) {
	PearCLDiver *pdcl = thread->pdcl;
	CLContext *cl = &(pdcl->cl);
	cl_command_queue *cmdq = &(cl->clcmdq[thread->index]);
	cl_mem *mem = cl->buffers[thread->index];
	BufferInfo *bufinfo = cl->kernel.buffer;

#ifdef DEBUG
	fprintf(stderr, "Writing buffers... ");
#endif
	clEnqueueWriteBuffer(*cmdq, mem[1], CL_TRUE, 0, bufinfo[1].size, 
			&(thread->states.mid_low), 0, NULL, NULL);
	clEnqueueWriteBuffer(*cmdq, mem[2], CL_TRUE, 0, bufinfo[2].size, 
			&(thread->states.mid_high), 0, NULL, NULL);
	clEnqueueWriteBuffer(*cmdq, mem[5], CL_TRUE, 0, bufinfo[5].size,
			&(thread->min_weight_magnitude), 0, NULL, NULL);
	clEnqueueWriteBuffer(*cmdq, mem[8], CL_TRUE, 0, bufinfo[8].size,
			&(pdcl->loop_count), 0, NULL, NULL);
#ifdef DEBUG
	fprintf(stderr, "Buffers Written.\n");
#endif
	/*
	   fprintf(stderr, "E: failed to write mid state low");
	   fprintf(stderr, "E: failed to write mid state low");
	   fprintf(stderr, "E: failed to write min_weight_magnitude");
	   fprintf(stderr, "E: failed to write min_weight_magnitude");
	   */
}
#if defined(_WIN32) && !defined(__MINGW32__)
DWORD WINAPI pearcl_find(void *data) {
#else
	void *pearcl_find(void *data) {
#endif
		size_t local_work_size,
			   global_work_size,
			   global_offset = 0,
			   num_groups;
		char found = 0;
		cl_event ev, ev1;
		PDCLThread *thread;
		PearCLDiver *pdcl;
		thread = (PDCLThread *)data;
		pdcl = thread->pdcl;
		num_groups = (pdcl->cl.num_cores[thread->index]);
		local_work_size = STATE_LENGTH;
		while (local_work_size > pdcl->cl.num_multiple[thread->index]) {
			local_work_size /= 3;
		}
		global_work_size = local_work_size * num_groups;

		for (int i = 0; i < thread->index; i++) {
			global_offset += pdcl->cl.num_cores[i];
		}
		pearcl_write_buffers(thread);

#ifdef DEBUG
		fprintf(stderr, "Initializing cores... ");
#endif
		if(CL_SUCCESS == 
				clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.clkernel[thread->index][0], 1, &global_offset,
					&global_work_size, &local_work_size, 0, NULL, &ev)
		  ) {
			clWaitForEvents(1, &ev);
			clReleaseEvent(ev);
#ifdef DEBUG
			fprintf(stderr, "Done.\nRunning search kernels...");
#endif

			while (found == 0 && pdcl->pd.status == PD_SEARCHING) {
				if(
						clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index],
							pdcl->cl.clkernel[thread->index][1], 1, NULL,
							&global_work_size, &local_work_size, 0, NULL, &ev1)
						!= CL_SUCCESS) {
					clReleaseEvent(ev1);
					fprintf(stderr, "E: running search kernel failed. \n");
					break;
				}
				clWaitForEvents(1, &ev1);
				clReleaseEvent(ev1);
				if(CL_SUCCESS != 
						clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
							pdcl->cl.buffers[thread->index][6], CL_TRUE, 0,
							sizeof(char), &found, 0, NULL, NULL)) {
					fprintf(stderr, "E: reading finished bool failed.\n");
					break;
				}
			}
		} else {
			fprintf(stderr, "E: running init kernel failed.\n" );
			clReleaseEvent(ev);
		}

#ifdef DEBUG
		fprintf(stderr, "Done.\nFinalizing...");
#endif
		if(CL_SUCCESS != 
				clEnqueueNDRangeKernel(pdcl->cl.clcmdq[thread->index],
					pdcl->cl.clkernel[thread->index][2], 1, NULL,
					&global_work_size, &local_work_size, 0, NULL, &ev))
			fprintf(stderr, "E: running finalize kernel failed.\n");

#ifdef DEBUG
		fprintf(stderr, "Finalize finished.\n");
#endif
		if (found > 0) {
			pthread_mutex_lock(&pdcl->pd.new_thread_search);
			if (pdcl->pd.status != PD_FOUND) {
				pdcl->pd.status = PD_FOUND;
#ifdef DEBUG
				fprintf(stderr, "Reading output trits...\n");
#endif
				if(CL_SUCCESS != 
						clEnqueueReadBuffer(pdcl->cl.clcmdq[thread->index],
							pdcl->cl.buffers[thread->index][0], CL_TRUE,
							0, HASH_LENGTH * sizeof(trit_t), &(thread->trits[TRANSACTION_LENGTH - HASH_LENGTH]),
							1, &ev, NULL)) {
#ifdef DEBUG
					fprintf(stderr, "E: reading transaction hash failed.\n");
#endif
				}
#ifdef DEBUG
				fprintf(stderr, "output trit read done.\n");
#endif
			}
			pthread_mutex_unlock(&pdcl->pd.new_thread_search);
		}
		clReleaseEvent(ev);
		return 0;
	}

	void pearcl_search(
			PearCLDiver *pdcl,
			trit_t *const trits,
			size_t length,
			size_t min_weight_magnitude
			) {
		int k, thread_count;
		int numberOfThreads = pdcl->cl.num_devices;

		if (length != TRANSACTION_LENGTH || min_weight_magnitude > HASH_LENGTH) {
			pdcl->pd.status = PD_INVALID;
#ifdef DEBUG
			fprintf(stderr, "E: Invalid transaction length. Got %lu, expected %lu\n",
					length, (size_t)TRANSACTION_LENGTH);
#endif
			return;
			/*
			   return Invalid_transaction_trits_length;
			   return Invalid_min_weight_magnitude;
			   */
		}


		pdcl->pd.status = PD_SEARCHING;

		States states;
		pd_search_init(&states, trits);

		if (numberOfThreads == 0) {
			pdcl->pd.status = PD_FAILED;
			return;
		}


		pthread_mutex_init(&pdcl->pd.new_thread_search, NULL);
		pthread_t *tid = malloc(numberOfThreads * sizeof(pthread_t));
		thread_count = numberOfThreads;

		PDCLThread  *pdthreads = (PDCLThread  *)malloc(numberOfThreads * sizeof(PDCLThread));
#ifdef DEBUG
		fprintf(stderr, "Starting CL Threads. \n");
#endif
		while (numberOfThreads-- > 0) {
			pdthreads[numberOfThreads] = (PDCLThread) {
				.states = states,
					.trits = trits,
					.min_weight_magnitude = min_weight_magnitude,
					.index = numberOfThreads,
					.pdcl = pdcl
			};
			pthread_create(&tid[numberOfThreads], NULL, &pearcl_find,
					(void *)&(pdthreads[numberOfThreads]));
		}

#ifdef DEBUG
		fprintf(stderr, "Waiting for CL Threads to finish. \n");
#endif
		sched_yield();
		for (k = thread_count; k > 0; k--) {
			pthread_join(tid[k - 1], NULL);
		}

		pthread_mutex_destroy(&pdcl->pd.new_thread_search);
#ifdef DEBUG
		fprintf(stderr, "Returning to master. Status is %u \n", pdcl->pd.status);
#endif
		free(tid);
		free(pdthreads);
		//return pdcl->pd.interrupted;
	}

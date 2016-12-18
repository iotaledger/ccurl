__constant size_t bit_size_of_long_long = sizeof(long)*8;

void increment(__global trit_t *states, int from, int to) {
	__private size_t id = get_local_id(0)*3 ;
	__private size_t gid = get_global_id(0) / HASH_LENGTH;
	__private size_t midLow = gid*STATE_LENGTH*4,
			  midHigh = midLow + STATE_LENGTH;
	size_t i,j;
#pragma unroll
	for(i=0; i<3; i++) {
		j = id + i;
		if( j >= from || j < to) {
			if (states[midLow + j] == LOW_BITS) {
				states[midLow + j] = HIGH_BITS;
				states[midHigh +j] = LOW_BITS;
			} else {
				if (states[midHigh +j] == LOW_BITS) {
					states[midHigh +j] = HIGH_BITS;
				} else {
					states[midLow + j] = LOW_BITS;
				}
			}
		}
	}
}

void cl_transform (
		__global trit_t *states
		) {
	size_t i, j,round;
	__private size_t id = get_local_id(0)*3;
	__private size_t gid = get_global_id(0)/HASH_LENGTH;// get_global_offset(0);
	__private trit_t alpha, beta, gamma, delta;
	__private size_t midLow = gid*STATE_LENGTH*4,
			  midHigh = midLow + STATE_LENGTH,
			  low = midHigh + STATE_LENGTH, 
			  high = low + STATE_LENGTH;
	increment(states,(HASH_LENGTH / 3) * 2,HASH_LENGTH);
	__private trit_t scratchpadLow[3];
	__private trit_t scratchpadHigh[3];
	__private size_t l1,l2;
	//barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
	for (i = 0; i < 3; i++) {
		j = id + i;
		states[low + j] = states[midLow + j];
		states[high + j] = states[midHigh + j];
	}

#pragma unroll
	for (round = 27; round-- > 0; ) {
		barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
		for (i = 0; i < 3; i++) {
			j = id + i;
			l1 = j == 0? 0:(((j - 1)>>1)+1)*HALF_LENGTH - ((j-1)>>1);
			l2 = ((j>>1)+1)*HALF_LENGTH - (j>>1);
			alpha = states[low + l1];
			beta = states[high + l1];
			gamma = states[high + l2];
			delta = (alpha | (~gamma)) & (states[low + l2] ^ beta);

			scratchpadLow[i] = ~delta;
			scratchpadHigh[i] = (alpha ^ gamma) | delta;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
		for (i = 0; i < 3; i++) {
			j = id + i;
			states[low + j] = scratchpadLow[i];
			states[high + j] = scratchpadHigh[i];
		}
	}
}

void cl_check (
		__constant size_t *minWeightMagnitude,
		__global trit_t *states,
		__local volatile size_t *found
		) {
	__private size_t id = get_local_id(0)*3 ; 
	__private size_t i, j,k;
	__private size_t gid = get_global_id(0);
	__private size_t midLow = gid*STATE_LENGTH*4,
			  midHigh = midLow + STATE_LENGTH,
			  low = midHigh + STATE_LENGTH, 
			  high = low + STATE_LENGTH;
	//if(id == 0 && gid == 0) printf("From Check 0, Hello!\n");
#pragma unroll
	for(i = 0; i < 64; i++ ) {
		if(id == 0) *found = 0;
		barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
		for (k = 0; k < 3; k++) {
			j = id + k;
			if(*found !=0) break;
			if (j >= HASH_LENGTH - *minWeightMagnitude && 
					((trit_t)states[low + j] & (1 << i)) != 
					((trit_t)states[high + j] & (1 << i))) {
				barrier(CLK_LOCAL_MEM_FENCE);
				*found = -1;
			}
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		if(*found == 0) {
			if(id == 0) *found = j;
			return;
		}
	}
	*found = -1;
}

void cl_finalize (
			__global trit_t *trits,
			__global volatile char *finished,
			__global volatile unsigned long *found,
			__global trit_t *states,
			__local unsigned long *mask,
			__local volatile size_t *bitIndex
		) {
	__private size_t j,k;
	__private size_t id = get_local_id(0)*3;
	__private size_t gid = get_global_id(0)/HASH_LENGTH;
	__private size_t midLow = gid*STATE_LENGTH*4,
			  midHigh = midLow + STATE_LENGTH,
			  low = midHigh + STATE_LENGTH, 
			  high = low + STATE_LENGTH;
	if(!*found & *mask || (*found << (bit_size_of_long_long - gid)) != 0)
		return;
	//if(id == 0 && gid == 0) printf("From Finalize 0, Hello!\n");
#pragma unroll
	for(k = 0; k < 3; k++) {
		j = id + k;
		if(j < HASH_LENGTH)
			trits[j] = (states[midLow + j] & (1<< *bitIndex)) == 0 ? 
				1 : (states[midHigh + j] & (1<< *bitIndex)) == 0 ? -1 : 0;
	}
	if(id == 0) {
		*finished = 1;
	}
}

__kernel void init (
		__global trit_t *trits,
		__global unsigned long *found,
		__global char *finished,
		__global trit_t *states,
		__constant size_t *minWeightMagnitude,
		//__local size_t *indices,
		__local unsigned long *mask,
		__local size_t *bitIndex
		) {
	__private int my_id = get_local_id(0);
	if(my_id >= HASH_LENGTH) return;
	size_t i, j;
	__private size_t id = my_id*3;
	__private size_t gid = get_global_id(0)/HASH_LENGTH;
	__private size_t midLow = gid*STATE_LENGTH*4,
			  midHigh = midLow + STATE_LENGTH,
			  low = midHigh + STATE_LENGTH, 
			  high = low + STATE_LENGTH;

#pragma unroll
	for(i = 0; i < 3; i++) {
		j = id + i;
		states[midLow + j] = states[j]; //midstatelow
		states[midHigh + j] = states[j + STATE_LENGTH]; //midstatehigh
	}
	for (i = gid; i-- > 0; ) {
		increment (states, HASH_LENGTH / 3, (HASH_LENGTH / 3) * 2);
	}

	if(id == 0) {
		*mask = 1;
		*mask <<= gid;
		//printf("\nMask for gid %d: %#010x", gid, *mask);
	}
}
	/*
#pragma unroll
	for(i = 0; i < 3; i++) {
		j = id + i;
		indices[j*2]   = j == 0? 0:(((j - 1)>>1)+1)*HALF_LENGTH - ((j-1)>>1);
		indices[j*2+1] = ((j>>1)+1)*HALF_LENGTH - (j>>1);
		if(indices[j*2] >= STATE_LENGTH || indices[j*2+1] >=STATE_LENGTH) {
			*found = 2;
			return;
		}
	}
	*/

	__kernel void search (
			__global trit_t *trits,
			__global volatile unsigned long *found,
			__global char *finished,
			__global trit_t *states,
			__constant size_t *minWeightMagnitude,
			//__local size_t *indices,
			__local unsigned long *mask,
			__local volatile size_t *bitIndex
			) {
		__private size_t i,j,k, id, gid, midLow, midHigh, low, high;
		id = get_local_id(0)*3;
		gid = get_global_id(0)/HASH_LENGTH;
		midLow = gid*STATE_LENGTH*4;
		midHigh = midLow + STATE_LENGTH;
		low = midHigh + STATE_LENGTH; 
		high = low + STATE_LENGTH;
		__local trit_t scratchpadLow[STATE_LENGTH]; 
		__local trit_t scratchpadHigh[STATE_LENGTH];
		//if(id == 0) printf("\nMask for gid in search %d: %#010x", gid, *mask);
#pragma unroll
		for(i = 0; i < sizeof(size_t); i++) { 
			if(*found != 0) return;
			cl_transform(states);
			cl_check(minWeightMagnitude, states, bitIndex);
			barrier(CLK_LOCAL_MEM_FENCE);
			if (*bitIndex != -1){
				if(id == 0) {
					*found |= *mask;
				}
				break;
			}
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
		cl_finalize(trits, finished, found, states, mask, bitIndex);
	}



/*

__kernel void transform (
		__global trit_t *trits,
		__global unsigned long *found,
		__global char *finished,
		__global trit_t *states,
		__constant size_t *minWeightMagnitude,
		__local size_t *indices,
		__local unsigned long *mask,
		__local size_t *bitIndex
		) {
	__private size_t my_id = get_local_id(0);
	if(my_id >= HASH_LENGTH) return;
	cl_transform(states);

	/ *
	   size_t i, j,round;
	   __private size_t id = my_id*3;
	   __private size_t gid = get_global_id(0)/HASH_LENGTH;
	   __private size_t midLow = gid*STATE_LENGTH*4,
	   midHigh = midLow + STATE_LENGTH,
	   low = midHigh + STATE_LENGTH, 
	   high = low + STATE_LENGTH;
	//if(id == 0 && gid == 0) printf("hello from trans kernel\n");
	__local trit_t scratchpadLow[STATE_LENGTH]; 
	__local trit_t scratchpadHigh[STATE_LENGTH];
	__private trit_t alpha, beta, gamma, delta;

	increment (states,(HASH_LENGTH / 3) * 2,HASH_LENGTH);

	barrier(CLK_LOCAL_MEM_FENCE);

#pragma unroll
for (i = 0; i < 3; i++) {
j = id + i;
states[low + j] = states[midLow + j];
states[high + j] = states[midHigh + j];
}

	// transform //
#pragma unroll
for (round = 27; round-- > 0; ) {
barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
for (i = 0; i < 3; i++) {
j = id + i;
scratchpadLow [j] = states[low + j];
scratchpadHigh[j] = states[high + j];
}
barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
for (i = 0; i < 3; i++) {
j = id + i;
alpha = scratchpadLow[indices[2*j]];
beta = scratchpadHigh[indices[2*j]];
gamma = scratchpadHigh[indices[2*j+1]];
delta = (alpha | (~gamma)) & (scratchpadLow[indices[2*j+1]] ^ beta);

states[low + j] = ~delta;
states[high + j] = (alpha ^ gamma) | delta;
}
}
* /
}
__kernel void check (
		__global trit_t *trits,
		__global volatile unsigned long *found,
		__global char *finished,
		__global trit_t *states,
		__constant size_t *minWeightMagnitude,
		__local size_t *indices,
		__local unsigned long *mask,
		__local volatile size_t *bitIndex
		) {
	__private size_t my_id = get_local_id(0);
	if(my_id >= HASH_LENGTH) return;
	__private size_t id = my_id*3 ; 
	__private size_t gid = get_global_id(0)/HASH_LENGTH;
	__private size_t midLow = gid*STATE_LENGTH*4,
			  midHigh = midLow + STATE_LENGTH,
			  low = midHigh + STATE_LENGTH, 
			  high = low + STATE_LENGTH;

	__private int i, j,k;
	__local bool skip;
	//if(id == 0 && get_global_id(0) == 0) printf("hello from check kernel\n");
#pragma unroll
	for(i = 0; i < 64; i++ ) {
		if(id == 0) {
			skip = false;
			*found |= *mask;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
		for (k = 0; k < 3; k++) {
			j = id + k;
			//printf("I've CHeckin IT! %#010x:%ld\n", id, get_global_id(0));
			if (j >= HASH_LENGTH - *minWeightMagnitude && 
					((trit_t)states[low + j] & (1 << i)) != 
					((trit_t)states[high + j] & (1 << i))) {
				*found &= ~*mask;
				skip = true;
			}
		}
		//barrier(CLK_LOCAL_MEM_FENCE);
		barrier(CLK_GLOBAL_MEM_FENCE);
		if(!skip){
			//if(){
			if(id == 0 && *found & *mask) {
				*finished = 1;
				//printf("Bitindex: %d\n", i);
				*bitIndex = i;
			}
			break;
		}
		//barrier(CLK_LOCAL_MEM_FENCE);
		}
	}
	__kernel void finalize (
			__global trit_t *trits,
			__global unsigned long *found,
			__global char *finished,
			__global trit_t *states,
			__constant size_t *minWeightMagnitude,
			__local size_t *indices,
			__local unsigned long *mask,
			__local size_t *bitIndex
			) {
		__private size_t my_id = get_local_id(0);
		if(my_id >= HASH_LENGTH) return;
		cl_finalize(trits, finished, found, states, mask, bitIndex);
	}
*/
/*
   __local trit_t *midStateLow, states[(gid+0)*STATE_LENGTH*4]
   __local trit_t *midStateHigh,states[(gid+1)*STATE_LENGTH*4]
   __local trit_t *stateLow,    states[(gid+2)*STATE_LENGTH*4]
   __local trit_t *stateHigh,   states[(gid+3)*STATE_LENGTH*4]
   */

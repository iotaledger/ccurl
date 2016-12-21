/*
 * PearlDiver
 *
 * Host copies state out to states
 * Host runs init kernel
 * 	kernel copes states to their array space
 * Host runs search kernel
 * 	search kernel searches sizeof(size_t) times for a nonce
 * 		If a nonce is found, it sets the bit index to the group's array
 *
 * 		successful kernel writes trits out to stateLow space ( midLow +
 * 		STATELENGTH*4)
 *
 * 		successful kernel sets finished to 1
 * Host reads finished variable
 * 	If finished is not 1, then host runs the search kernel again
 * Host reads bit index array, chooses the first group that has a bit index
 * array greater than or equal to 0
 *
 * Host reads trit hash (HASH_LENGTH) from that kernel's stateLow space
 *
 */

//__constant size_t bit_size_of_long_long = sizeof(long)*8;

void set_ids(__private size_t *id, __private size_t *gid) {
	*id = get_local_id(0)*3;
	*gid = get_global_id(0)/HASH_LENGTH;
}

void cl_find_end(
		__global trit_t *mid_low,
		__private size_t id,
		__local size_t *end,
		__private int from,
		__private int to
		) {
	int i,j;
	if(id == 0) {
		*end = to;
		for(i=from; i < to; i++) {
			if(mid_low[j] == LOW_BITS ) {
				*end = i;
				break;
			}
		}
	}
	barrier(CLK_LOCAL_MEM_FENCE);
}
void cl_increment(
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		int from,
		int to, 
		__private size_t id 
		) {
	__private size_t i,j;
	barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
	for(i=0; i<3; i++) {
		j = id + i;
		if( j >= from && j < to) {
			if (mid_low[j] == LOW_BITS) {
				mid_low[j] = HIGH_BITS;
				mid_high[j] = LOW_BITS;
			} else {
				if (mid_high[j] == LOW_BITS) {
					mid_high[j] = HIGH_BITS;
				} else {
					mid_low[j] = LOW_BITS;
				}
			}
		}
	}
	barrier(CLK_LOCAL_MEM_FENCE);
}

void cl_transform (
		__global trit_t *state_low,
		__global trit_t *state_high
		) {
	__private size_t id, gid;
	set_ids(&id, &gid);

	int i, j,round, l1, l2;
	__private trit_t alpha, beta, gamma, delta;
	__private trit_t scratchpadLow[3], scratchpadHigh[3];

#pragma unroll
	for (round = 27; round-- > 0; ) {
		barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
		for (i = 0; i < 3; i++) {
			j = id + i;
			l1 = j == 0? 0:(((j - 1)%2)+1)*HALF_LENGTH - ((j-1)>>1);
			l2 = ((j%2)+1)*HALF_LENGTH - ((j)>>1);
			alpha = state_low[l1];
			beta = state_high[l1];
			gamma = state_high[l2];
			delta = (alpha | (~gamma)) & (state_low[l2] ^ beta);

			scratchpadLow[i] = ~delta;
			scratchpadHigh[i] = (alpha ^ gamma) | delta;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
#pragma unroll
		for (i = 0; i < 3; i++) {
			j = id + i;
			state_low[j] = scratchpadLow[i];
			state_high[j] = scratchpadHigh[i];
		}
	}
}

void check_nonce(
		__global trit_t *state_low,
		__global trit_t *state_high,
		__constant size_t *min_weight_magnitude,
		__global trit_t *nonce_probe
		) {
	__private size_t nonce_start, nonce_offset, j, k;
	*nonce_probe = HIGH_BITS;
	for(j = HASH_LENGTH - *min_weight_magnitude - 1; j < HASH_LENGTH; j++) {
		*nonce_probe &= ~(state_low[j] ^ state_high[j]);
		if(*nonce_probe == 0)
		{
			break;
		}
	}
}

__kernel void finalize (
		__global trit_t *trit_hash,
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		__global trit_t *state_low,
		__global trit_t *state_high,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_probe//,__local size_t *increment_end
		
		) {
	__private size_t i,j,k, id, gid, start;
	set_ids(&id, &gid);
	gid = *found - 1;
	start = gid * STATE_LENGTH;
	if(id == 0 && nonce_probe[gid] > 0) {
		printf("No Zero NONCE!\n");
		printf("nonce probe: %ld\n", nonce_probe[gid]);
		printf("start: %d\n", (int)start);
	}
#pragma unroll
	for(k = 0; k < 3; k++) {
		j = id + k;
		if(j < HASH_LENGTH) {
			trit_hash[j] = (mid_low[start+j] & nonce_probe[gid]) == 0 ? 
				1 : (mid_high[start+j] & nonce_probe[gid]) == 0 ? -1 : 0;
		}
	}
}

__kernel void search (
		__global trit_t *trit_hash,
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		__global trit_t *state_low,
		__global trit_t *state_high,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_array//,__local size_t *increment_end
		) {
	__private size_t i,j,k, id, gid, start;
	__local size_t end;
	set_ids(&id, &gid);
	start = gid * STATE_LENGTH;
#pragma unroll
	for(i = 0; i < (size_t)1; i++) { 
		if(*found != 0){
			break;
		}
		cl_find_end(mid_low + start, id, &end, (HASH_LENGTH / 3) * 2, HASH_LENGTH);
		cl_increment(mid_low + start, mid_high + start,
				(HASH_LENGTH / 3) * 2, end + 1, id);
#pragma unroll
		for (j = 0; j < 3; j++) {
			k = id + i;
			state_low[start + k] = mid_low[start + k];
			state_high[start + k] = mid_high[start + k];
		}

		cl_transform(state_low + start, state_high + start);
		barrier(CLK_LOCAL_MEM_FENCE);
		if(id == 0) {
			check_nonce(state_low + start, state_high + start, min_weight_magnitude, &(nonce_array[gid]));
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		if (nonce_array[gid] != 0){
			if(id == 0 && *found == 0) {
				*found = 1 + gid;
			}
			break;
		}
	}
}

__kernel void init (
		__global trit_t *trit_hash,
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		__global trit_t *state_low,
		__global trit_t *state_high,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_probe//,__local size_t *increment_end
		) {
	__private size_t i, j, id, gid, offset;
	__local size_t end;
	set_ids(&id, &gid);
	offset = gid * STATE_LENGTH;

	if(id==0 && gid == 0) {
		*found = 0;
	}

	if(gid != 0) {
#pragma unroll
		for(i = 0; i < 3; i++) {
			j = id + i;
			mid_low[offset + j] = mid_low[j];
			mid_high[offset + j] = mid_high[j];
		}
	}
	cl_find_end(mid_low + offset, id, &end, HASH_LENGTH / 3, (HASH_LENGTH / 3) * 2);
	for (i = gid; i-- > 0; ) {
		cl_increment(mid_low + offset, mid_high + offset, 
				HASH_LENGTH / 3, end + 1, id);
	}
}

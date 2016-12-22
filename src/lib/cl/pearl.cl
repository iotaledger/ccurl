#define HASH_LENGTH 243
#define STATE_LENGTH 3 * HASH_LENGTH
#define HALF_LENGTH 364
#define HIGH_BITS 0b1111111111111111111111111111111111111111111111111111111111111111L
#define LOW_BITS 0b0000000000000000000000000000000000000000000000000000000000000000L

typedef long trit_t;

void increment(
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		__private size_t from_index,
		__private size_t to_index
		) {
	int i;
	for (i = from_index; i < to_index; i++) {
		if (mid_low[i] == (trit_t)LOW_BITS) {
			mid_low[i] = (trit_t)HIGH_BITS;
			mid_high[i] = (trit_t)LOW_BITS;
		} else {
			if (mid_high[i] == (trit_t)LOW_BITS) {
				mid_high[i] = (trit_t)HIGH_BITS;
			} else {
				mid_low[i] = (trit_t)LOW_BITS;
			}
			break;
		}
	}
}

void copy_mid_to_state(
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		__global trit_t *state_low,
		__global trit_t *state_high,
		__private size_t id
		) {
	int i, j;
#pragma unroll
	for(i = 0; i < 3; i++) {
		j = id + i;
		state_low[j] = mid_low[j];
		state_high[j] = mid_high[j];
	}
}

void transform(
		__global trit_t *state_low,
		__global trit_t *state_high,
		__private size_t id
		) {
	__private int round, i, j, t1, t2;
	__private trit_t alpha, beta, gamma, delta, sp_low[3], sp_high[3];
#pragma unroll
	for(round = 0; round < 27; round++) {
		for(i = 0; i < 3; i++) {
			j = id + i;
			t1 = j == 0? 0:(((j - 1)%2)+1)*HALF_LENGTH - ((j-1)>>1);
			t2 = ((j%2)+1)*HALF_LENGTH - ((j)>>1);

			alpha = state_low[t1];
			beta = state_high[t1];
			gamma = state_high[t2];
			delta = (alpha | (~gamma)) & (state_low[t2] ^ beta);

			sp_low[i] = ~delta;
			sp_high[i] = (alpha ^ gamma) | delta;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		for(i = 0; i < 3; i++) {
			j = id + i;
			state_low[j] = sp_low[i];
			state_high[j] = sp_high[i];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
}

void check(
		__global trit_t *state_low,
		__global trit_t *state_high,
		__global volatile char *found,
		__constant size_t *min_weight_magnitude,
		__global trit_t *nonce_probe,
		__private size_t gr_id
		) {
	int i;
	*nonce_probe = HIGH_BITS;
	for (i = HASH_LENGTH - *min_weight_magnitude; i < HASH_LENGTH; i++) {
		*nonce_probe &= ~(state_low[i] ^ state_high[i]);
		if(*nonce_probe == 0) return;
	}
	if(*nonce_probe != 0) *found = gr_id + 1;
}

__kernel void init (
		__global trit_t *trit_hash,
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		__global trit_t *state_low,
		__global trit_t *state_high,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_probe
		) {
	__private size_t i, j, id, gid, gr_id;
	id = get_local_id(0) * 3;
	gr_id = get_global_id(0)/HASH_LENGTH;
	gid = gr_id*STATE_LENGTH;
	if(id == 0 && gid == 0) *found = 0;

	if(gid == 0) return;

	for(i = 0; i < 3; i++) {
		j = id + i;
		mid_low[gid + j] = mid_low[j];
		mid_high[gid + j] = mid_high[j];
	}

	if(id == 0) {
		for(i = 0; i < gid; i++) {
			increment(&(mid_low[gid]), &(mid_high[gid]), HASH_LENGTH / 3, (HASH_LENGTH / 3) * 2);
		}
	}
	return;
}

__kernel void search (
		__global trit_t *trit_hash,
		__global trit_t *mid_low,
		__global trit_t *mid_high,
		__global trit_t *state_low,
		__global trit_t *state_high,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_probe
		) {
	__private size_t i,j,k, id, gid, gr_id;
	id = get_local_id(0) * 3;
	gr_id = get_global_id(0)/HASH_LENGTH;
	gid = gr_id*STATE_LENGTH;

	for(i = 0; i < 2048; i++) {
		if(id == 0) increment(&(mid_low[gid]), &(mid_high[gid]), (HASH_LENGTH/3)*2, HASH_LENGTH);

		barrier(CLK_LOCAL_MEM_FENCE);
		copy_mid_to_state(&(mid_low[gid]), &(mid_high[gid]), &(state_low[gid]), &(state_high[gid]), id);

		barrier(CLK_LOCAL_MEM_FENCE);
		transform(&(state_low[gid]), &(state_high[gid]), id);

		barrier(CLK_LOCAL_MEM_FENCE);
		if(id == 0) check(&(state_low[gid]), &(state_high[gid]), found, min_weight_magnitude, &(nonce_probe[gr_id]), gr_id);

		barrier(CLK_LOCAL_MEM_FENCE);
		if(*found != 0) break;
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
		__global trit_t *nonce_probe
		) {
	__private size_t i,j, id, gid, gr_id;
	id = get_local_id(0) * 3;
	gr_id = get_global_id(0)/HASH_LENGTH;
	gid = gr_id*STATE_LENGTH;
	if(gr_id != *found - 1) return;
	if(nonce_probe[gr_id] == 0 ) return;
#pragma unroll
	for(i = 0; i < 3; i++) {
		j = id + i;
		if(j < HASH_LENGTH) {
			trit_hash[j] = (mid_low[gid + j] & nonce_probe[gr_id]) == 0 ? 
				1 : (mid_high[gid + j] & nonce_probe[gr_id]) == 0 ? -1 : 0;
		}
	}
}

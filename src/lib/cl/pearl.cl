#define HASH_LENGTH 243
#define STATE_LENGTH 3 * HASH_LENGTH
#define HALF_LENGTH 364
#define HIGH_BITS 0b1111111111111111111111111111111111111111111111111111111111111111L
#define LOW_BITS 0b0000000000000000000000000000000000000000000000000000000000000000L

typedef long trit_t;

typedef struct {
	trit_t mid_low[HASH_LENGTH];
	trit_t mid_high[HASH_LENGTH];
	trit_t low[HASH_LENGTH];
	trit_t high[HASH_LENGTH];
} States;

void increment(
		__global States *state,
		__private size_t from_index,
		__private size_t to_index
		) {
	int i;
	for (i = from_index; i < to_index; i++) {
		if (state->mid_low[i] == LOW_BITS) {
			state->mid_low[i] = HIGH_BITS;
			state->mid_high[i] = LOW_BITS;
		} else {
			if (state->mid_high[i] == LOW_BITS) {
				state->mid_high[i] = HIGH_BITS;
			} else {
				state->mid_low[i] = LOW_BITS;
			}
			break;
		}
	}
}

void copy_mid_to_state(
		__global States *state,
		__private size_t id
		) {
	int i, j;
#pragma unroll
	for(i = 0; i < 3; i++) {
		j = id + i;
		state->low[j] = state->mid_low[j];
		state->high[j] = state->mid_high[j];
	}
}

void transform(
		__global States *state,
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

			alpha = state->low[t1];
			beta = state->high[t1];
			gamma = state->high[t2];
			delta = (alpha | (~gamma)) & (state->low[t2] ^ beta);

			sp_low[i] = ~delta;
			sp_high[i] = (alpha ^ gamma) | delta;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
		for(i = 0; i < 3; i++) {
			j = id + i;
			state->low[j] = sp_low[i];
			state->high[j] = sp_high[i];
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
}

void check(
		__global States *state,
		__global volatile char *found,
		__constant size_t *min_weight_magnitude,
		__global trit_t *nonce_probe,
		__private size_t gid
		) {
	int i;
	*nonce_probe = HIGH_BITS;
	for (i = HASH_LENGTH - *min_weight_magnitude; i < HASH_LENGTH; i++) {
		*nonce_probe &= ~(state->low[i] ^ state->high[i]);
		if(*nonce_probe == 0) return;
	}
	if(*nonce_probe != 0) *found = gid + 1;
}

__kernel void init (
		__global trit_t *trit_hash,
		__global States *states,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_probe
		) {
	__private size_t i, j, id, gid;
	id = get_local_id(0) * 3;
	gid = get_global_id(0)/HASH_LENGTH;
	if(id == 0 && gid == 0) *found = 0;

	if(gid == 0) return;

	for(i = 0; i < 3; i++) {
		j = id + i;
		states[gid].mid_low[j] = states[0].mid_low[j];
		states[gid].mid_high[j] = states[0].mid_high[j];
	}

	if(id != 0) return;

	for(i = 0; i < gid; i++) {
		increment(&(states[gid]), HASH_LENGTH / 3, (HASH_LENGTH / 3) * 2);
	}
}

__kernel void search (
		__global trit_t *trit_hash,
		__global States *states,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_probe
		) {
	__private size_t i,j,k, id, gid;
	id = get_local_id(0) * 3;
	gid = get_global_id(0)/HASH_LENGTH;

	__global States *state = &(states[gid]);

	for(i = 0; i < 1024; i++) {
		if(id == 0) increment(state, (HASH_LENGTH/3)*2, HASH_LENGTH);

		barrier(CLK_LOCAL_MEM_FENCE);
		copy_mid_to_state(state, id);

		barrier(CLK_LOCAL_MEM_FENCE);
		transform(state, id);

		barrier(CLK_LOCAL_MEM_FENCE);
		if(id == 0) check(state, found, min_weight_magnitude, &(nonce_probe[gid]), gid);

		barrier(CLK_LOCAL_MEM_FENCE);
		if(*found != 0) break;
	}
}


__kernel void finalize (
		__global trit_t *trit_hash,
		__global States *states,
		__constant size_t *min_weight_magnitude,
		__global volatile char *found,
		__global trit_t *nonce_probe
		) {
	__private size_t i,j, id, gid;
	id = get_local_id(0) * 3;
	gid = get_global_id(0) / HASH_LENGTH;
	if(gid != *found - 1) return;
	if(nonce_probe[gid] == 0 ) return;
#pragma unroll
	for(i = 0; i < 3; i++) {
		j = id + i;
		if(j < HASH_LENGTH) {
			trit_hash[j] = (states[gid].mid_low[j] & nonce_probe[gid]) == 0 ? 
				1 : (states[gid].mid_high[j] & nonce_probe[gid]) == 0 ? -1 : 0;
		}
	}
}

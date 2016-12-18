#include "ccurl.h"
#include "../lib/PearlDiver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRYTE_LENGTH 2673

#define HINTS "### CCURL ###\nUsage:\n\tccurl-cli <MinWeightMagnitude> [TRYTES (length: %d)] \n\techo TRYTES | ccurl-cli <MinWeightMagnitude>\n"


int get_stdin(char *str, int len) {
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);

	fd_set savefds = readfds;

	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	char chr;
	int i = 0;
	if (select(1, &readfds, NULL, NULL, &timeout)) {
		puts("Input:");
		while ((chr = getchar()) != EOF) {
			if(i>=len) return -1;
			str[i++] = chr;
		}
	}
	readfds = savefds;
	//str[i] = 0;
	return i;
}

void trytes2trits(trit_t *trits, const char *trytes, const size_t len) {
	for(size_t i=0; i<len; i++) {
		size_t idx = (trytes[i]=='9' ? 0 : trytes[i]-'A'+1);
		trits[3*i+0] = iotacurl_tryte2trits_tbl[idx][0];
		trits[3*i+1] = iotacurl_tryte2trits_tbl[idx][1];
		trits[3*i+2] = iotacurl_tryte2trits_tbl[idx][2];
	}
}

void trits2trytes(char *trytes, const trit_t *trits, const size_t len) {
	for(size_t i=0; i<len; i+=3) {
		int j = trits[i];
		if(i+1 < len) {
			j += 3 * trits[i+1];
		}
		if(i+2 < len) {
			j += 9 * trits[i+2];
		}
		if(j < 0) {
			j += 27;
		}
		trytes[i/3] = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ"[j];
	}
}

int main(int argc, char *argv[]) {
	char buf[TRYTE_LENGTH];
	trit_t trits[TRANSACTION_LENGTH];
	PearlDiver pearl_diver;
	long minWeightMagnitude;

	if(argc < 2 ) {
		fprintf(stderr, HINTS, TRYTE_LENGTH); 
		return 1;
	}

	fprintf(stderr, "args... %d", argc);
	if(argc > 2) {
		switch(strlen(argv[2])) {
			case TRYTE_LENGTH:
				memcpy(buf,argv[2],sizeof(char)*TRYTE_LENGTH);
				break;
			default:
				if(get_stdin(buf, TRYTE_LENGTH) != TRYTE_LENGTH) {
					fprintf(stderr, HINTS, TRYTE_LENGTH); 
					return 1;
				}
		}
	} else if(get_stdin(buf, TRYTE_LENGTH) != TRYTE_LENGTH) {
		fprintf(stderr, HINTS, TRYTE_LENGTH); 
		return 1;
	}
	minWeightMagnitude = atol(argv[1]);
	if( minWeightMagnitude == 0 ) {
		fprintf(stderr, HINTS, TRYTE_LENGTH); 
		return 1;
	}

	trytes2trits(trits, buf, TRYTE_LENGTH);

	fprintf(stderr, "Starting...");
	//search(&pearl_diver, trits, TRANSACTION_LENGTH, (int)minWeightMagnitude, -1); //use all threads
	pd_search(&pearl_diver, trits, TRANSACTION_LENGTH, (int)minWeightMagnitude, 1);

	trits2trytes(buf, trits, TRANSACTION_LENGTH);

	*(buf + TRYTE_LENGTH) = 0;
	printf("%s",buf);
	return 0;
}

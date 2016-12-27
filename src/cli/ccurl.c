#include "../lib/ccurl.h"
#include "../lib/Hash.h"
#include "../lib/PearlDiver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "Windows.h"
/*
#include "Winsock2.h"
#pragma comment(lib, "Ws2_32.lib")
*/
#else
#include <sys/select.h>
#endif



#define TRYTE_LENGTH 2673

#define HINTS "### CCURL ###\nUsage:\n\tccurl-cli <MinWeightMagnitude> [TRYTES (length: %d)] \n\techo TRYTES | ccurl-cli <MinWeightMagnitude>\n"


int get_stdin(char *str, int len) {

	int i = 0;
	char chr;
	struct timeval timeout;
	fd_set readfds, savefds;
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);

	savefds = readfds;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

#ifdef _WIN32
	if (select(1, &readfds, NULL, NULL, &timeout)) {
#else
	if (select(1, &readfds, NULL, NULL, &timeout)) {
#endif
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

int main(int argc, char *argv[]) {
	char buf[TRYTE_LENGTH], *output, *hash;
	long minWeightMagnitude;

	if(argc < 2 ) {
		fprintf(stderr, HINTS, TRYTE_LENGTH); 
		return 1;
	}

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
	output = ccurl_pow(buf, minWeightMagnitude);
	hash = ccurl_digest_transaction(output);
	printf("OUTPUT:\n%s", output);
	printf("\nHash:\n%s", hash);
	return 0;
}
//trit_t trits[TRANSACTION_LENGTH];
//PearlDiver pearl_diver;
	/*
	init_converter();
	Curl curl;
	absorb(&curl, trits_from_trytes(buf, TRYTE_LENGTH), 0, TRYTE_LENGTH);
	trit_t hash[HASH_LENGTH];
	squeeze(&curl, hash, 0, HASH_LENGTH);
	printf("\nHASH:\n%s", trytes_from_trits(hash, 0, HASH_LENGTH));
	*/

#include "../lib/ccurl.h"
#include "../lib/Hash.h"
#include "../lib/PearlDiver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
#include "Winsock2.h"
#else
#include <sys/select.h>
#endif



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

int main(int argc, char *argv[]) {
	char buf[TRYTE_LENGTH];
	//trit_t trits[TRANSACTION_LENGTH];
	//PearlDiver pearl_diver;
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

	printf("%s",ccurl_pow(buf, minWeightMagnitude));
	return 0;
}

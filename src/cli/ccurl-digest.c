#include "../lib/ccurl.h"
#include "../lib/Hash.h"
#include "../lib/PearlDiver.h"
#include "../lib/util/converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
#include "Windows.h"
#include "Winsock2.h"
#pragma comment(lib, "Ws2_32.lib")
*/
#ifndef _WIN32
//#include <sys/select.h>
#else
#define STDIN_FILENO 0
#endif



#define TRYTE_LENGTH 2673

#define HINTS "### CCURL DIGEST ###\nUsage:\n\tccurl-cli [TRYTES (length: %d)] \n\techo TRYTES | ccurl-cli \n"


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

	//if (select(1, &readfds, NULL, NULL, &timeout)) {
	if (select(1, &readfds, NULL, NULL, NULL)) {
		while ((chr = getchar()) != EOF) {
			if (i > len) return -1;
			str[i++] = chr;
		}
	}
	readfds = savefds;
	str[i] = 0;
	return i;
}

int main(int argc, char *argv[]) {
	char *buf;
	int in_size;

	if (argc > 1) {
		if (strlen(argv[1]) >= TRYTE_LENGTH) {
			buf = argv[1];
			//memcpy(buf, argv[1], sizeof(char)*TRYTE_LENGTH);
		}
		else {
			fprintf(stderr, HINTS, TRYTE_LENGTH);
			return 1;
		}
	} else {
		buf = malloc(sizeof(char)*(TRYTE_LENGTH + 1));
		in_size = get_stdin(buf, TRYTE_LENGTH);
		if (in_size != TRYTE_LENGTH) {
			fprintf(stderr, HINTS, TRYTE_LENGTH);
			return 1;
		}
	}
	fputs(ccurl_digest_transaction(buf),stderr);
	return 0;
}

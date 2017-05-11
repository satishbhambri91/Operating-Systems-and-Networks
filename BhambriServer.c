
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>


#define MAX_PLL_THRDS 5
#define SIZEOFFILENAME 50
#define SIZEOFFILE 1024
#define ASZE 3
#define FF "420"
#define FNF "421"



struct ThrArgs {
	int nsockfd;
	int* count_of_thrds;
};

struct Thread {
	pthread_t id;
	struct ThrArgs arguments;
};

void error(const char *message) {
	perror(message);
	exit(1);
}

int kr;
static void* client_thread(void* arguments) {
	struct ThrArgs* _args = (struct ThrArgs*) arguments;
	char* itr;
	FILE* file;
	int n;
	long fileSize;
	char fbuffer[SIZEOFFILE];
	char fnme[SIZEOFFILENAME];


	printf("\nClient thread has been instantiated.\n");
	if (_args->nsockfd < 0) {
		error("Client cannot be accepted.\n");
	}
	bzero(fnme, SIZEOFFILENAME);
	n = read(_args->nsockfd, fnme, SIZEOFFILENAME);
	if (n < 0) {
		error("Not able to read from socket.\n");
	}
	printf("Thread %d: Opening the file : %s\n", _args->nsockfd,
			fnme);


	if (access(fnme, R_OK) != -1) {
		bzero(fbuffer, SIZEOFFILE);
		file = fopen(fnme, "rb");
		fseek(file, 0, SEEK_END);
		fileSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		fread(fbuffer, SIZEOFFILE, 1, file);
		fclose(file);

		n = write(_args->nsockfd, FF, strlen(FF));
		if (n < 0) {
			error("Unable to write to the socket.\n");
		}

		n = write(_args->nsockfd, fbuffer, strlen(fbuffer));
		if (n < 0) {
			error("Unable to write to the socket.\n");
		}

	} else {
		n = write(_args->nsockfd, FNF, strlen(FNF));
		if (n < 0) {
			error("Unable to write to the socket.\n");
		}
	}
	printf("Closing the thread.\n");
	*_args->count_of_thrds -= 1;
	close(_args->nsockfd);
}

int main(int argc, char *argv[]) {
	int sockfd, nsockfd, port, count_of_thrds;
	socklen_t clength;
	struct sockaddr_in server_address, client_address;
	struct ThrArgs* arguments;

	if (argc < 2) {
		fprintf(stderr, "port not specified.\n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("Unable to open the socket.\n");
	}

	bzero((char *) &server_address, sizeof(server_address));
	port = atoi(argv[1]);

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *) &server_address,
			sizeof(server_address)) < 0) {
		error("Unable to bind the socket.\n");
	}

	listen(sockfd, MAX_PLL_THRDS);
	clength = sizeof(client_address);

	kr = 1;
	count_of_thrds = 0;

	while (1) {
		if (count_of_thrds < MAX_PLL_THRDS) {
			pthread_t id;
			nsockfd = accept(sockfd, (struct sockaddr *) &client_address,
					&clength);
			arguments = (struct ThrArgs*) malloc(
					sizeof(struct ThrArgs));
			arguments->nsockfd = nsockfd;
			arguments->count_of_thrds = &count_of_thrds;
			pthread_create(&id, NULL, &client_thread, arguments);
			count_of_thrds += 1;
		}

	}
	printf("\nClosing the server, Thank you \n");
	close(sockfd);
	return 0;
}

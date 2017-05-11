#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define ASZE 4
#define FF "420"
#define FNF "421"
#define SIZEOFFILENAME 50
#define SIZEOFFILE 1024


void error(const char *message) {
	perror(message);
	exit(0);
}

int main(int argc, char *argv[]) {
	int sockfd;
	struct hostent *srvr;
	int portNumber;
	int n;
	struct sockaddr_in srvraddr;


	char fbffr[SIZEOFFILE];
	char fname[SIZEOFFILENAME];
	char acknowledgementbffr[ASZE];

	if (argc < 2) {
		fprintf(stderr, "usage %s port\n", argv[0]);
		exit(0);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	portNumber = atoi(argv[1]);

	if (sockfd < 0) {
		error("Unable to open the socket.\n");
	}

	srvr = gethostbyname("localhost");

	if (srvr == NULL) {
		fprintf(stderr, "Unable to locate the host\n");
		exit(0);
	}

	bzero((char *) &srvraddr, sizeof(srvraddr));
	srvraddr.sin_family = AF_INET;
	bcopy((char *) srvr->h_addr, (char *)&srvraddr.sin_addr.s_addr, srvr->h_length);
	srvraddr.sin_port = htons(portNumber);

	if (connect(sockfd, (struct sockaddr *) &srvraddr,
			sizeof(srvraddr)) < 0) {
		error("Unable to connect to the server.\n");
	}

	printf("Connected to the Server !\n");
	printf("Please enter the file name: ");
	bzero(fname, SIZEOFFILENAME);
	fgets(fname, SIZEOFFILENAME - 1, stdin);

	if (fname[strlen(fname) - 1] == '\n') {
		fname[strlen(fname) - 1] = 0;
	}

	n = write(sockfd, fname, strlen(fname));
	if (n < 0) {
		error("Unable to write to the socket.\n");
	}


	bzero(acknowledgementbffr, 4);
	n = read(sockfd, acknowledgementbffr, 3);
	if (n < 0) {
		error("Unable to read from the socket.\n");
	}


	if (strcmp(acknowledgementbffr, FF) == 0) {
		bzero(fbffr, SIZEOFFILE);
		printf("File found!.\n");
		printf("Receiving the file content.\n");
		n = read(sockfd, fbffr, SIZEOFFILE);
		if (n < 0) {
			error("Unable to read from socket.\n");
		}
		printf("\n---Begin File Content----\n%s\n\---End File Content---\n",
				fbffr);

	} else if (strcmp(acknowledgementbffr, FNF) == 0) {
		printf("Server is unable to locate the file.\n");
	} else {
		printf("Unidentified error.\n");
	}
	printf("Closing the client, Than you. \n");
	close(sockfd);
	return 0;
}

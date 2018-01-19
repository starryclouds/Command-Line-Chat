// Sonali Chaudhry
// Based on COEN 146 Lab 3
// TalkTo.c (based on client.c) - threaded version

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

void *send_thread (void *x);
void *recv_thread (void *x);

char buffer[500], myname[500], servername[500];
int serversize;

struct hostent *server;
// server struct

struct sockaddr_in si_server;
// structure of server/how to connect to it

int sockfd;

int main (int argc, char *argv[]) {
	if (argc < 2) {
		printf ("not enough arguments\n");
		return 0;
	}
	//argv[1] = hostname
	strcpy(buffer, "");

	server = gethostbyname(argv[1]);
	// convert server name (www.kdjf, localhost, etc) to IP address

	bzero((char*) &si_server, sizeof(si_server));
	// clears si_server so any leftover data is gone

	si_server.sin_family = AF_INET;

	bcopy((char*) server->h_addr, (char*) &si_server.sin_addr, server->h_length);

	si_server.sin_port = htons(5632);
	// assigns the port number

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	// creates sockets
	if (sockfd < 0)
		printf("socket error\n");

	serversize = sizeof(si_server);

	printf ("Please enter a username:\n");
	fgets (myname, 500, stdin);

	int i = 0;
	while (myname[i] != '\n') {
		i++;
	}
	myname[i] = '\0';

	int er = sendto(sockfd, myname, strlen(myname)+1, 0, (struct sockaddr*) &si_server, sizeof(si_server));
	if (er < 0)
		printf("sendto error\n");

	int len = recvfrom(sockfd, servername, 500, 0, (struct sockaddr*) &si_server, &serversize);
	if (len < 0)
		printf("recvfrom error\n");
	servername[len] = '\0';

	printf ("You may now begin chatting.\n");


	pthread_t sendt, recvt;
	if (pthread_create(&sendt, NULL, send_thread, (void *) &serversize) < 0) {
		printf ("pthread_create error");
		return 1;
	}
	if (pthread_create(&recvt, NULL, recv_thread, (void *) &serversize) < 0) {
		printf ("pthread_create error");
		return 1;
	}
	if (pthread_join(recvt, NULL)) {
		printf ("pthread_join error");
		return 1;
	}

	return 0;
}

void *send_thread (void *x) {
	while (strcmp(buffer, "quit\n") != 0) {
		char buf[500];
		fgets (buf, 500, stdin);
		int er = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*) &si_server, sizeof(si_server));
		if (er < 0)
			printf("sendto error\n");

		if (strcmp(buf, "quit\n") == 0) {
			strcpy(buffer, "quit\n");
			pthread_exit(NULL);
			break;
		}
	}
	return NULL;
}

void *recv_thread (void *x) {
	while (strcmp(buffer, "quit\n") != 0) {
		char buf[500];
		serversize = sizeof(si_server);
		int len = recvfrom(sockfd, buf, 500, 0, (struct sockaddr*) &si_server, &serversize);
		if (len < 0) {
			printf("recvfrom error\n");
			break;
		}

		buf[len] = '\0';

		if (strcmp(buf, "quit\n") == 0) {
			strcpy(buffer, "quit\n");
			int er = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*) &si_server, sizeof(si_server));
			if (er < 0)
				printf("sendto error\n");
			pthread_exit(NULL);
			break;
		}

		printf ("%s: %s", servername, buf);
	}	
	return NULL;
}

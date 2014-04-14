#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_LINE 1024
#define CTRL_D 4

char usage[66] = "usage: snc [-k] [-l] [-u] [-s source_ip_address] [hostname] port\n";
char error[16] = "internal error\n";
int kFlag = 0;
int lFlag = 0;
int uFlag = 0;
int sFlag = 0;
char *src_ip, *hostname, *str_port;
int port;


pthread_t *in_data = (pthread_t*)malloc(sizeof(pthread_t));
pthread_t *out_data = (pthread_t *)malloc(sizeof(pthread_t));

int parseFlags(int argc, char** argv);
int parseArgs(int argc, char** argv);
int isNumeric(char *str);
int isIP(char *str);
void  *sendOutput(void * arg);
void *getInput(void * arg);
int sockfd;
struct sockaddr_in sin;
struct sockaddr_in * recvaddr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in)); // struct to recieve UDP sender info
int addrlen = sizeof(struct sockaddr_in);
int rcvdfromclient = 0; // 1 if we are server and client has already sent us a message
struct hostent *hp, *sp;

int main(int argc, char** argv) {
	// handle user input
	parseArgs(argc, argv);

	// establish socket connection
	// if UDP
	if (uFlag) 
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	else // if TCP
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf(error);
		exit(0);
	}
	printf("Socket Established\n");
	
	int rc, clientlen, connfd;
	struct sockaddr_in clientaddr;

	// WE ARE THE SERVER
	if (lFlag) {
		// build hostname data structure
		bzero((char *)&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		
		if(!hostname) {
			sin.sin_addr.s_addr = INADDR_ANY;
		} else {
			hp = gethostbyname(hostname);
			if (!hp) {
				printf(error);
				exit(0);
			}
			bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
			sin.sin_addr.s_addr = inet_addr(hostname);
		}
		
		if (bind(sockfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == -1) {
			printf(error);
			printf("binding socket failed\n");
			exit(0);
		}

		// need to listen and accept for TCP
		if (!uFlag) {
			rc = listen(sockfd, 1); // 1 is backlog value

			if (rc < 0) {
				printf(error);
				exit(0);
			}

			char recvbuf[MAX_LINE];

			// keep listening for connections if k flag is specified
			if (kFlag) {
				while (1) {
					//clientlen = sizeof(sockaddr_in);	
					if ((connfd = accept(sockfd, (sockaddr*)&sin, (socklen_t*)&clientlen)) < 0) {
						printf(error);
						exit(0);
					}
					pthread_create(out_data, NULL, sendOutput,  &connfd );
					pthread_create(in_data, NULL, getInput, &connfd );
				}
			}
			// otherwise only accept the first connection
			else {
				printf("No k flag!\n");
				//clientlen = sizeof(sockaddr_in);
				//connfd = accept(sockfd, (sockaddr*)&clientaddr, (socklen_t*)&clientlen);
				if ((connfd = accept(sockfd, (sockaddr*)&sin, (socklen_t*)&clientlen)) < 0) {
					printf(error);
					exit(0);
				}
				pthread_create(out_data, NULL, sendOutput, &connfd );
				pthread_create(in_data, NULL, getInput, &connfd );
			}

		// UDP Packets
		}else{

				pthread_create(out_data, NULL, sendOutput, &sockfd );
				pthread_create(in_data, NULL, getInput, &sockfd );

		}//end UCP (server)

	} else { // WE ARE THE CLIENT
		
		hp = gethostbyname(hostname);
		if(!hp) {
			printf(error);
			exit(0);
		}
		
		// build hostname data structure
		bzero((char *)&sin, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
		sin.sin_addr.s_addr = htonl(inet_addr(hostname));
		
		if (sFlag) {
					
			//verify ip exists
			if (src_ip == NULL) {
				printf(error);
				exit(0);
			}

			// build src_ip data structure
			struct sockaddr_in srcipsin;
			bzero((char *)&srcipsin, sizeof(srcipsin));
			srcipsin.sin_family = AF_INET;
			srcipsin.sin_port = htons(port);
			
			sp = gethostbyname(src_ip);
			
			bcopy(sp->h_addr, (char*)&srcipsin.sin_addr, sp->h_length);
			srcipsin.sin_addr.s_addr = inet_addr(src_ip);

			// bind to specified interface
			if (bind(sockfd, (struct sockaddr *)&srcipsin, sizeof(struct sockaddr_in)) == -1) {
				printf(error);
				printf("binding socket failed\n");
				exit(0);
			}
		}	
		
		// connect
		if (connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) == -1) {
			printf(error);
			printf("client connection failed\n");
			exit(0);
		}
		

		pthread_create(out_data, NULL,  sendOutput, &sockfd );
		pthread_create(in_data, NULL,  getInput,  &sockfd );


	}

	pthread_join(*in_data, NULL);
	pthread_join(*out_data, NULL);
	printf("exiting snc\n");
	return 0;

}

/**
 * Function: getInput(int connfd)
 *
 * Listens for input from the specified socket until ctrl + D is entered
 */
void * getInput(void * arg) {
	int *connfd = (int *)arg;
	char recvbuf[MAX_LINE];
	int rc;
	printf("connfd: %d\n", *connfd);
	if (uFlag) {
		while(recvfrom(*connfd, recvbuf, sizeof(recvbuf), 0, (sockaddr *)recvaddr, (socklen_t*)&addrlen)) {
			fputs(recvbuf, stdout);
			if (lFlag)
				rcvdfromclient = 1;
		}
	} else {
		while(recv(*connfd, recvbuf, sizeof(recvbuf), 0)) {
			fputs(recvbuf, stdout);
		}
	}
	printf("Closing connection due to no input\n");
	close(*connfd);
	pthread_cancel(*out_data);
}

/***
 * Function: sendOutput(int sockfd)
 *
 *  Sends output until ctrl + D is entered
 */
void * sendOutput( void * arg) {
	int * sockfd =(int * ) arg;
	int sendbuflen;
	char sendbuf[MAX_LINE];
	printf("sockfd: %d\n", *sockfd);
	while (fgets(sendbuf, sizeof(sendbuf), stdin)) {
		sendbuf[MAX_LINE - 1] = '\0';
		sendbuflen = strlen(sendbuf) + 1;
		
		if (uFlag) {
			
			//server needs to have recieved a packet from client before sending	
			if(lFlag){
				while(!rcvdfromclient){
				   printf("I don't know where to send, silly!\n");
				   pthread_yield();
				}
				int rc = sendto(*sockfd, sendbuf, sendbuflen, 0, (sockaddr *)recvaddr, (socklen_t )addrlen);	
				printf("server--bytes sendto: %u\n", rc);
			}
			//client already knows where it wants to send
			else{
				int sz_sin = sizeof(sin);
				int rc = sendto(*sockfd, sendbuf, sendbuflen, 0, (sockaddr *)&sin, (socklen_t )sz_sin);	
				printf("client --bytes sendto: %u\n", rc);
			}
		} else {
			send(*sockfd, sendbuf, sendbuflen, 0);
		}
	}
	printf("closing connection due to ctrl + D\n");
	rcvdfromclient = 0; // reset var when connection is shut down
	//close(*sockfd);
	// cancel the receiving thread unless UDP is being used
	if (!uFlag) {
		printf("cancelling receiving thread\n");
		close(*sockfd);
		pthread_cancel(*in_data);
	}
}


/***
 * Function: parseArgs(int argc, char** argv)
 *
 * Parses user input. Prints an error message and exits when an
 * error is encountered.
 */
int parseArgs(int argc, char** argv) {

	if (argc < 3 || argc > 8) {
		printf(usage);
		exit(0);
	}	

	int rc = parseFlags(argc, argv);
	printf("rc: %d\n", rc);
	if (rc < 0) {
		printf(usage);
		exit(0);
	}
	
	// check if the number of args and flags makes sense
	if (sFlag && ((argc - rc) > 3)) {
		printf(usage);
		exit(0);
	} else if ((argc - rc) > 2) {
		printf(usage);
		exit(0);
	}

	// check non-flag arguments
	if (lFlag) {
		// this is last arg
		if((rc + 1) == argc) {
			// must be a port -- save it
			str_port = argv[rc];
		}
		else {
			hostname = argv[rc];
			str_port = argv[rc + 1];
		}
	}
	else {
		if((rc + 1) == argc) {
			// if this is the last arg there is an error
			printf(usage);
			exit(0);
		}
		else {
			hostname = argv[rc];
			str_port = argv[rc + 1];
		}
	}

	// check the port
	if (!isNumeric(str_port)) {
		printf("port: %s\n", str_port);
		printf(usage);
		exit(0);
	}
	// check the IP
	if ((src_ip != NULL) && !isIP(src_ip)) {
		printf("src_ip: %s\n", src_ip);
		printf(usage);
		exit(0);
	}
	port = atoi(str_port);


	printf("IP Addr: %s\n", src_ip);
	printf("Hostname: %s\n", hostname);
	printf("Port: %d\n", port);
	printf("kflag: %d\nlflag: %d\nuflag: %d\nsflag: %d\n", kFlag, lFlag, uFlag, sFlag);
	return 0;
}

/***
 * Function: isNumeric(char *str)
 *
 * Checks if all of the characters in the given string are digits.  If they
 * are, returns 1.  Else returns 0;
 */
int isNumeric(char *str) {
	
	while (*str) {
		if (!isdigit(*str))
			return 0;
		str++;
	}

	return 1;

}

/****
 * Function isIP(char *str)
 *
 * Checks if all of the characters in the given string are either digits or periods.
 * If they are, returns 1.  Else returns 0.
 */
int isIP(char *str) {
	
	while (*str) {
		if ( (!isdigit(*str)) && (*str != 46))
			return 0;
		str++;
	}

	return 1;

}

/****
 * Function: parseFlags(int argc, char** argv[])
 * 
 * Reads char* from the command until one is reached that is not a flag.
 * Returns the position of the next argument (the first argument that is
 * not a flag) or -1 if there is an invalid flag combination.
 */
int parseFlags(int argc, char** argv) {

	int i;
	for (i = 1; i < argc; i++) {
		printf("%s\n", argv[i]);
		if ( strcmp(argv[i], "-k") == 0 ) {
			kFlag = 1;
		}
		else if (strcmp(argv[i], "-l") == 0) {
			lFlag = 1;
		}
		else if (strcmp(argv[i], "-u") == 0) {
			uFlag = 1;
		}
		else if (strcmp(argv[i], "-s") == 0) {
			// if not followed by src_ip return error
			if ((i + 1) == argc) {
				return -1;
			}
			src_ip = argv[i + 1];
			sFlag = 1;
			i++;
		} else {
			break;
		}
	}
	if ( (!lFlag && kFlag) || (sFlag && lFlag) ) {
		// invalid flag combination
		return -1;
	}

	return i;

}

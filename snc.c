#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

int parseFlags(int argc, char** argv);
int parseArgs(int argc, char** argv);
int isNumeric(char *str);
int isIP(char *str);

int main(int argc, char** argv) {
	// handle user input
	parseArgs(argc, argv);

	// establish socket connection
	int sockfd;
	struct sockaddr_in sin;

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
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		sin.sin_addr.s_addr = INADDR_ANY;

		if (bind(sockfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == -1) {
			printf(error);
			printf("binding socket failed\n");
			exit(0);
		}

		// need to listen and accept for TCP
		if (!uFlag) {
			rc = listen(sockfd, 5);

			if (rc < 0) {
				printf(error);
				exit(0);
			}
			// keep listening for connections if k flag is specified
			if (kFlag) {
				while (1) {
					clientlen = sizeof(sockaddr_in);	
					connfd = accept(sockfd, (sockaddr*)&clientaddr, (socklen_t*)&clientlen);
				}
			}
			// otherwise only accept the first connection
			else {
				clientlen = sizeof(sockaddr_in);
				connfd = accept(sockfd, (sockaddr*)&clientaddr, (socklen_t*)&clientlen);
			}
	
		}


	} else { // WE ARE THE CLIENT
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
		if (sFlag) {
			sin.sin_addr.s_addr = inet_addr(src_ip);
		} else {
			sin.sin_addr.s_addr = inet_addr(hostname);
		}
		/*if (connect(sockfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) == -1) {
			printf(error);
			printf("connection failed\n");
			exit(0);
		}*/

	}

	// read from stdin until enter or EOF (Ctrl ^ D) is reached
	char *buffer = (char *) malloc(MAX_LINE);
	char *check = (char *) malloc(MAX_LINE);
	int exit = 0;
	do {
		check = fgets(buffer, MAX_LINE, stdin);
		printf("first char: %c\n", *buffer);
		if (check == NULL) {
			// check lFlag and kFLag to see if it should remain open
			exit = 1;
			printf("CTRL + D!\n");
		}
		else {
			printf("Read: %s\n", buffer);
		}

	} while (!exit);


	// bind the socket
	//sin = (sockaddr_in*)malloc(sizeof(struct sockaddr_in)); 
	/*sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	printf("src_ip: %s\n", src_ip);
	sin.sin_addr.s_addr = inet_addr();
	//inet_pton(AF_INET, src_ip, &((sin->sin_addr).s_addr));

	if(bind(sockfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in) ) == -1)
	{
		    printf(error);
		    printf("binding socket failed\n");
		    exit(0);
	}*/
	
	printf("exiting snc\n");
	return 0;

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

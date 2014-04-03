#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char usage[66] = "Usage: snc [-k] [-l] [-u] [-s source_ip_address] [hostname] port\n";
int kFlag = 0;
int lFlag = 0;
int uFlag = 0;
int sFlag = 0;
char *src_ip, *hostname; 
int port;

int parseFlags(int argc, char** argv);

int main(int argc, char** argv) {

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

	// done with flags
	if (lFlag) {
		// this is last arg
		if((rc + 1) == argc) {
			// must be a port -- save it
			port = atoi(argv[rc]);
		}
		else {
			hostname = argv[rc];
			port = atoi(argv[rc + 1]);
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
			port = atoi(argv[rc + 1]);
		}
	}
	printf("IP Addr: %s\n", src_ip);
	printf("Hostname: %s\n", hostname);
	printf("Port: %d\n", port);
	printf("kflag: %d\nlflag: %d\nuflag: %d\nsflag: %d\n", kFlag, lFlag, uFlag, sFlag);
	return 0;

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

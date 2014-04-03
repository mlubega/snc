#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char usage[66] = "Usage: snc [-k] [-l] [-u] [-s source_ip_address] [hostname] port\n";
int kFlag = 0;
int lFlag = 0;
int uFlag = 0;
int sFlag = 0;

int main(int argc, char** argv) {

	if (argc < 3 || argc > 8) {
		printf(usage);
		exit(0);
	}	
	
	int i;
	for (i = 1; i < argc; i++) {
		printf(argv[i]);
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
			// is it followed by src_ip?
		}
		else {
			// done with flags
		}

	}
	
	
	return 0;

	

}


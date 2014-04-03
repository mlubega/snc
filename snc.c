#include <stdio.h>
#include <stdlib.h>

char usage[62] = "Usage: [-k] [-l] [-u] [-s source_ip_address] [hostname] port\n";

int main(int argc, char** argv) {

	if (argc < 3 || argc > 8) {
		printf(usage);
	}	

	return 0;
	
}

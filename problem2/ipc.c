#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <stdlib.h>

static void
usage(void){
	fprintf(stderr,"usage: ipc [ -r maxrate/sec]\n");
}

// Parent launches some group of children, and exchanges messages with them
// at some prescribed rate. We provide a FIFO (pipe) solution; another method
// would be to use shared-memory and process-shared mutex locks.
int main(int argc,char **argv){
	unsigned long long rate = 0; // per second; 0 = unlimited
	int c;

	while((c = getopt(argc,argv,"r:")) != -1){
		switch(c){
			case 'r':{
				char *e;

				if(rate){
					usage();
					return EXIT_FAILURE;
				}
				if((rate = strtoull(optarg,&e,0)) == 0 || rate == ULLONG_MAX){
					usage();
					return EXIT_FAILURE;
				}
				break;}
			case '?':
			default:
				usage();
				return EXIT_FAILURE;
		}
	}
	if(argv[optind]){
		usage();
		return EXIT_FAILURE;
	}
	if(rate){
		printf("Ratelimited to %llu message per second\n",rate);
	}else{
		printf("No ratelimit in effect\n");
	}
	return EXIT_SUCCESS;
}

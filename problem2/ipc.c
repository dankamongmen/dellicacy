#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CHILDCOUNT 10

static void
usage(void){
	fprintf(stderr,"usage: ipc [ -r maxrate/sec ]\n");
}

// Child routine. We read messages from the pipe, 
static void
suckle_server_teat(void){
}

// 
static int
feed_hungry_children(unsigned mouths,const int *pipes,unsigned long long rate){
	return 0;
}

static int
spawn_hungry_children(unsigned mouths,int **pipes){
	unsigned z;

	if(mouths == 0){
		return -1;
	}
	if((*pipes = malloc(sizeof(*pipes) * mouths)) == NULL){
		fprintf(stderr,"Couldn't allocate %u-pipe array (%s?)\n",
				mouths,strerror(errno));
		return -1;
	}
	for(z = 0 ; z < mouths ; ++z){
		int pfd[2]; // [0] is read (cli) end, [1] is write (srv) end
		pid_t pid;

		// On error, so long as we exit, all existing children will
		// see their pipes closed, causing SIGPIPE to be delivered.
		if(pipe(pfd)){
			fprintf(stderr,"Couldn't create FIFO IPC %u (%s?)\n",
					z,strerror(errno));
			free(*pipes);
			return -1;
		}
		if((pid = fork()) < 0){
			fprintf(stderr,"Error fork()ing child %u (%s?)\n",
					z,strerror(errno));
			free(*pipes);
			return -1;
		}else if(pid){
			printf("Spawned child %u at PID %jd\n",z,(uintmax_t)pid);
		}else{
			suckle_server_teat();
			exit(EXIT_SUCCESS);
		}
	}
	return 0;
}

// Parent launches some group of children, and exchanges messages with them
// at some prescribed rate. We provide a FIFO (pipe) solution; another method
// would be to use shared-memory and process-shared mutex locks.
int main(int argc,char **argv){
	unsigned mouths = CHILDCOUNT;
	unsigned long long rate = 0; // per second; 0 = unlimited
	int c,*pipes;

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
	printf("Serving %u %s\n",mouths,mouths == 1 ? "child" : "children");
	if(spawn_hungry_children(mouths,&pipes)){
		return EXIT_FAILURE;
	}
	if(feed_hungry_children(mouths,pipes,rate)){
		free(pipes);
		return EXIT_FAILURE;
	}
	free(pipes);
	return EXIT_SUCCESS;
}

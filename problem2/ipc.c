#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define CHILDCOUNT 10

static void
usage(void){
	fprintf(stderr,"usage: ipc [ -r maxrate/sec ]\n");
}

// Child routine. We read messages from the pipe. Each message is a pair of
// unsigned 64-bit integers, of which we must find the greatest common divisor.
static void
suckle_server_teat(int teat){
	char fn[PATH_MAX];
	uint64_t in[2];
	FILE *out;

	if(snprintf(fn,sizeof(fn),"%jd.out",(intmax_t)getpid()) >= sizeof(fn)){
		exit(EXIT_FAILURE);
	}
	if((out = fopen(fn,"w")) == NULL){
		fprintf(stderr,"Couldn't open %s (%s?)\n",fn,strerror(errno));
		exit(EXIT_FAILURE);
	}
	while(read(teat,in,sizeof(in)) == sizeof(in)){
		fprintf(stderr,"%ju %ju flipp\n",in[0],in[1]);
	}
	if(fclose(out)){
		fprintf(stderr,"Error closing %s (%s?)\n",fn,strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_FAILURE);
}

// Send one message to each child
static int
send_msgs(int rnd,unsigned mouths,const int *pipes){
	uint64_t buf[2 * mouths];

	if(read(rnd,buf,sizeof(buf)) != sizeof(buf)){
		fprintf(stderr,"Error reading %zu PRNG bytes\n",sizeof(buf));
		return -1;
	}
	while(mouths--){
		if(write(pipes[mouths],buf + mouths * 2,sizeof(*buf) * 2) != sizeof(*buf) * 2){
			fprintf(stderr,"Error writing %zu to fd %d\n",
					sizeof(*buf) * 2,pipes[mouths]);
			return -1;
		}
	}
	return 0;
}

// Send messages to the children as quickly as possible.
static int
forcefeed_hungry_children(int rnd,unsigned mouths,const int *pipes){
	while(1){
		if(send_msgs(rnd,mouths,pipes)){
			return -1;
		}
	}
}

// Send messages to the children, no faster than the specified ratelimit (0
// for no limit) expressed in msgs/sec/pipe.
static int
feed_hungry_children(int rnd,unsigned mouths,const int *pipes,
				unsigned long long rate){
	unsigned long long bkt;

	if(rate == 0){
		return forcefeed_hungry_children(rnd,mouths,pipes);
	}
	while(1){
		struct timeval then;
		struct timespec ts;

		// Each iteration, get the current time. Send rate messages.
		// Check to see if we've exceeded a second. If so, loop back
		// immediately. Otherwise, pause for the remainder. We are
		// thus highly bursty when ratelimiting is being effected.
		gettimeofday(&then,NULL);
		bkt = rate;
		while(bkt--){
			if(send_msgs(rnd,mouths,pipes)){
				return -1;
			}
		}
		do{
			struct timeval now,diff;

			gettimeofday(&now,NULL);
			timersub(&then,&now,&diff);
			if(diff.tv_sec >= 1){
				break;
			}
			ts.tv_sec = 0;
			ts.tv_nsec = diff.tv_usec * 1000u;
		}while(nanosleep(&ts,NULL) && errno == EINTR);
	}
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
		(*pipes)[z] = pfd[1];
		fflush(stdout);
		if((pid = fork()) < 0){
			fprintf(stderr,"Error fork()ing child %u (%s?)\n",
					z,strerror(errno));
			free(*pipes);
			return -1;
		}else if(pid){
			printf("Spawned child %u at PID %jd\n",z,(uintmax_t)pid);
			close(pfd[0]);
		}else{
			close(pfd[1]);
			suckle_server_teat(pfd[0]);
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
	int c,*pipes,rnd;

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
	if((rnd = open("/dev/urandom",O_RDONLY)) < 0){
		fprintf(stderr,"Coudln't open /dev/urandom for input (%s?)\n",
				strerror(errno));
		return -1;
	}
	if(feed_hungry_children(rnd,mouths,pipes,rate)){
		close(rnd);
		free(pipes);
		return EXIT_FAILURE;
	}
	close(rnd);
	free(pipes);
	return EXIT_SUCCESS;
}

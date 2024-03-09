#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"

// shared  globals
unsigned int mode;
unsigned int size, num_threads;
double **A, **B, **SEQ_MATRIX, **PAR_MATRIX;

int main(int argc, char *argv[]) {

	// TODO - deal with command line arguments, save the "mode"
	// "size" and "num threads" into globals so threads can see them
	if(argc < 3) {
		printf("Usage: ./mmm S <size>\n");
		printf("USage: ./mmm P <threads> <size>\n");
		return 0;
	}
	if(strcmp(argv[1], "S") == 0) {
		if(argc != 3) {
			printf("Usage: ./mmm S <size>\n");
			return 0;
		}
		mode = 1;
		size = strtol(argv[2], NULL, 10);
		printf("========\n");
		printf("mode: sequential\n");
		printf("thread count: 1\n");
		printf("size: %d\n", size);
		printf("========\n");
	} else if(strcmp(argv[1], "P") == 0) {
		if(argc != 4) {
			printf("USage: ./mmm P <threads> <size>\n");
			return 0;
		}
		mode = 2;
		size = strtol(argv[3], NULL, 10);
		num_threads = strtol(argv[2], NULL, 10);
		if(num_threads > size) {
			printf("Requested too many threads.\n");
			return 0;
		}
		printf("========\n");
		printf("mode: parallel\n");
		printf("thread count: %d\n", num_threads);
		printf("size: %d\n", size);
		printf("========\n");
	} else {
		printf("Usage: ./mmm S <size>\n");
		printf("USage: ./mmm P <threads> <size>\n");
		return 0;
	}

	// initialize my matrices
	mmm_init();
	double seqClockstart, seqClockend, parClockstart, parClockend;
	// << stuff I want to clock here >>
	mmm_seq(); // throwaway run
	seqClockstart = rtclock();	// start the clock
	for(int i = 0; i < 3; i++) {
		mmm_seq(); // run sequential 3 times under timer
		mmm_reset(SEQ_MATRIX);
	}
	seqClockend = rtclock();
	if(mode == 2) { // throwaway run
		int i;
		pthread_t tid[num_threads];
		int *args = (int*) malloc(num_threads * sizeof(int));
		for (i = 0; i < num_threads; i++) { 
			args[i] = (size / num_threads) * i;
			pthread_create(&tid[i], NULL, mmm_par, &args[i]);
		}
		//wait for all the threads to finish
		for (i = 0; i < num_threads; i++) {
			pthread_join(tid[i], NULL);
		}
		free(args);		
		parClockstart = rtclock();	// start the clock
		for(int i = 0; i < 3; i++) {
			int i;
			pthread_t tid[num_threads];
			int *args = (int*) malloc(num_threads * sizeof(int));
			for (i = 0; i < num_threads; i++) { 
				args[i] = (size / num_threads) * i;
				pthread_create(&tid[i], NULL, mmm_par, &args[i]);
			}
			//wait for all the threads to finish
			for (i = 0; i < num_threads; i++) {
				pthread_join(tid[i], NULL);
			}
			free(args);	
			mmm_reset(PAR_MATRIX);
		}
		parClockend = rtclock();	
	}

	//clockend = rtclock(); // stop the clock
	
	printf("Sequential Time (avg of 3 runs): %.6f sec\n", (seqClockend - seqClockstart)/3);
	if(mode == 2) {
		printf("Parallel Time (avg of 3 runs): %.6f sec\n", (parClockend - parClockstart)/3);
		printf("Speedup: %f\n", ((seqClockend - seqClockstart)/(parClockend - parClockstart)));
		printf("Verifying... largest error between parallel and sequential matrix: %f\n", mmm_verify());
	}
	//display();
	// free some stuff up
	mmm_freeup();
	return 0;
}

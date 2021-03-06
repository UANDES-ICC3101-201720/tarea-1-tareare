#define _POSIX_C_SOURCE 199309L
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include "types.h"
#include "const.h"
#include "util.h"

pthread_mutex_t region_mutex = PTHREAD_MUTEX_INITIALIZER;


//funcion serial binary search
int serial_binsearch(int A[], int n, int T) {
	int L = 0;
	int R = n-1;
	int m;
	while (L <= R){
	    	m = ((L + R) / 2);
		if (A[m] < T)
		    L = m + 1;
		else if (A[m] > T)
		    R = m - 1;
		else		
		    return m;
	}
	return -1;    
}
struct final{
	int x;
	int y;
	int z[];
};

void *first(void * fv){
	struct final *fs = fv;
	pthread_exit((void *) (intptr_t) serial_binsearch(fs->z, fs->x, fs->y)); 
}
void *second(void * fv){
	struct final *fs = fv;
	pthread_exit((void *) (intptr_t) serial_binsearch(fs->z, fs->x, fs->y)); 
}
void *third(void * fv){
	struct final *fs = fv;
	pthread_exit((void *) (intptr_t) serial_binsearch(fs->z, fs->x, fs->y)); 
}
void *fourth(void * fv){
	struct final *fs = fv;
	pthread_exit((void *) (intptr_t) serial_binsearch(fs->z, fs->x, fs->y));
}
// funcion parallel binary search
int parallel_binsearch(int A[], int n, int T) {
	int B[n/3];
	
	int C[n/3];
	
	int D[n/3];

	for(int i=0; i<n/3; i++){B[i] = A[i]; } 
	for(int i=n/3; i<2*(n/3); i++){C[i-(n/3)] = A[i]; } 
	for(int i=2*(n/3); i<n; i++){D[i-(2*n/3)] = A[i]; } 	

	struct final *fi = malloc(sizeof(struct final)+(n/3)*sizeof(int));
	fi->x = n/3;
	fi->y = A[T];
	memcpy(fi->z, B, (n/3)*sizeof(int));
	
	struct final *se = malloc(sizeof(struct final)+(n/3)*sizeof(int));
	se->x = n/3;
	se->y = A[T];
	memcpy(se->z, C, (n/3)*sizeof(int));	

	struct final *th = malloc(sizeof(struct final)+(n/3)*sizeof(int));
	th->x = n/3;
	th->y = A[T];
	memcpy(th->z, D, (n/3)*sizeof(int));

	struct final *fo = malloc(sizeof(struct final)+(n/3)*sizeof(int));
	fo->x = n/3;
	fo->y = A[T];
	memcpy(fo->z, D, (n/3)*sizeof(int));

	pthread_t first_thread;
	pthread_t second_thread;
	pthread_t third_thread;
	pthread_t fourth_thread;
	
	pthread_create(&first_thread, NULL, first, fi);
	void * fv;
	pthread_join(first_thread, &fv);
	int q = (intptr_t) fv;	

	pthread_create(&second_thread, NULL, second, se);
	void * fq;
	pthread_join(second_thread, &fq);
	int w = (intptr_t) fq;

	pthread_create(&third_thread, NULL, third, th);
	void * fr;
	pthread_join(third_thread, &fr);
	int f = (intptr_t) fr;	
	
	pthread_create(&fourth_thread, NULL, fourth, fo);
	void * ft;
	pthread_join(fourth_thread, &ft);
	int p = (intptr_t) ft;	

	if(q >= 0){return q;}
	else if(w >= 0){return (w+333);}
	else if(f >= 0){return (f+666);}
	else if(p >= 0){return (p+999);}
	return -1;
}

int main(int argc, char** argv) {
	
	printf("[binsearch] Starting up...\n");

	printf("[binsearch] Number of cores available: '%ld'\n",
	   sysconf(_SC_NPROCESSORS_ONLN));

	char* Tvalue;
	int Evalue = 0;
	int Pvalue = 0;
	int index;
	int c;

	opterr = 0;


	while ((c = getopt (argc, argv, "E:T:P:")) != -1)
		switch (c){
			case 'E':
				Evalue = atoi(optarg);
				break;
			case 'T':
				Tvalue = optarg;
				break;
			case 'P':
				Pvalue = atoi(optarg);
				break;
			case '?':
				if (optopt == 'P' || optopt == 'T' || optopt == 'E')
				  fprintf (stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
				  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
				else
				  fprintf (stderr,
					   "Unknown option character `\\x%x'.\n",
					   optopt);
				return 1;
			default:
				abort ();
		}


	printf ("E = %d, T = %s, P = %d\n",
	  Evalue, Tvalue, Pvalue);

	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);


	pid_t pid;
	pid = fork();
	if (pid == -1){   
		fprintf(stderr, "Error en fork\n");
		exit(EXIT_FAILURE);
	}
	if (pid == 0){
		execlp("./datagen","./datagen",NULL);
	}

	struct sockaddr_un addr;
  	int fd,rc;
	char buf[10];
	strcpy(buf,"BEGIN S");
	strcat(buf, Tvalue);
	
	if ( (fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket error");
		exit(-1);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, DSOCKET_PATH, sizeof(addr.sun_path)-1);

	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("connect error");
		exit(-1);
	}
	if ((rc = write(fd, buf, sizeof(buf))) == -1){
		perror("write error\n");
		exit(-1);
	}
	printf("Enviando info: %d bytes enviados.\n", rc);

	int buf2[1000];
	int linea=0;
	int len=pow(10,atoi(Tvalue));
	int arreglo[len];
	while ((rc = read(fd, buf2, sizeof(buf2))) >= 1000){
		if (linea==0){
			for (int i=0;i<1000;i++){
				arreglo[i+linea*1000]=buf2[i+1];
			}
		}
		else{
			for (int i=0;i<1000;i++){
				arreglo[i+linea*1000]=buf2[i];
			}
		}
		linea++;
	}
	if ((rc = write(fd, "END", sizeof(buf))) == -1){
		perror("write error\n");
		exit(-1);
	}
	clock_t cbegin = clock();

	struct timespec start, finish;
	double elapsed_s = 0;
	double elapsed_p = 0;
	printf("E,T,TIEMPO SERIAL,TIEMPO PARALELO\n");

	for (int i=0; i<Evalue; i++){

		clock_gettime(CLOCK_MONOTONIC, &start);
		serial_binsearch(arreglo,len, arreglo[Pvalue]);
		clock_gettime(CLOCK_MONOTONIC, &finish);
		elapsed_s = (finish.tv_sec - start.tv_sec);
		elapsed_s += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

		clock_gettime(CLOCK_MONOTONIC, &start);
		parallel_binsearch(arreglo, len, Pvalue);
		clock_gettime(CLOCK_MONOTONIC, &finish);
		elapsed_p = (finish.tv_sec - start.tv_sec);
		elapsed_p += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
		printf("%d,%d,%lf,%lf\n", i+1, atoi(Tvalue), elapsed_s, elapsed_p);	
	}

	clock_t cend = clock();

	double time_elapsed = ((double) (cend - cbegin) / CLOCKS_PER_SEC) * 1000;

	printf("Time elapsed '%lf' [ms].\n", time_elapsed);

	exit(0);
}
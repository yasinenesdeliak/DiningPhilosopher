#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#define handle_error(msg)\
    do{perror(msg); exit(EXIT_FAILURE);} while(0)

typedef struct task {
   int i;
   char* dst;
   int min_think;
   int max_think;
   int min_dine;
   int max_dine;
   int numberOfDinners;
   int num_phsp;
} workerTask;

sem_t* forks;
sem_t* dinnerQuotaLock;


int uniform(int high, int low) {
    double random = rand()/(1.0 + RAND_MAX); 
    int range = high - low + 1;
    int random_scaled = (range * random) + low;
    return random_scaled;
}

double exponential(double lambda)
{
  double z;                     
  double e_value;             

  z = (double) uniform(1, 100) / 101.0;

  
  e_value = -lambda * log(z);
  return (int) e_value;
}

void workerThread(void *i) {
    struct timespec stime;
    struct timespec ftime;
    double waittime;
    
    workerTask *a = ((workerTask *) i);
	int think_time = 0;
	int dine_time = 0;

	if(a->dst == "uniform") {
		think_time = uniform(a->min_think, a->max_think);
		dine_time = uniform(a->min_dine, a->max_dine);
	} else {
		think_time = exponential((double) (a->min_think + a->max_think) / 2);
		dine_time = exponential( (double) ((a->min_dine + a->max_dine) / 2));
	}
clock_gettime(CLOCK_MONOTONIC, &stime);

	while(a->numberOfDinners > 0) {
		usleep(think_time*1000);


		sem_wait(&dinnerQuotaLock);
		sem_wait(&forks[a->i]);
		sem_wait(&forks[(a->i+1)%a->num_phsp]);

		a->numberOfDinners--;

		usleep(dine_time*1000);

		sem_post(&forks[a->i]);
		sem_post(&forks[(a->i+1)%a->num_phsp]);
		sem_post(&dinnerQuotaLock);
	}
	clock_gettime(CLOCK_MONOTONIC, &ftime);
	  waittime = (ftime.tv_sec - stime.tv_sec);
    waittime += (ftime.tv_nsec - stime.tv_nsec) / 1000000000.0;
    printf("Philosopher waited for %fms\n", waittime);
    printf("Philosopher %d is finish his job.\n", a->i);
    free(a);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	pid_t pid;
	int i;
	int num_phsp = atoi(argv[1]);
	  if(num_phsp > 27 || num_phsp < 1){
        handle_error("The maximum number of philosophers is 27");
    }else if(num_phsp%2==0){
        handle_error("The number of philosophers is not an odd number");
    }else if(num_phsp==1){
        handle_error("The number of philosophers cannot be 1.");
    }
	int min_think = atoi(argv[2]);
	int max_think = atoi(argv[3]);
	int min_dine = atoi(argv[4]);
	int max_dine = atoi(argv[5]);
	char* probDist = argv[6];
	int numberOfDinners = atoi(argv[7]);
	int tid[num_phsp];
	if(min_think<1 || min_dine<1){
        handle_error("Minimum Time can be 1ms");
    }
     if(max_think>60000 || max_dine > 60000){
        handle_error("Maximum Time can be 60s");
    }

	sem_t s[num_phsp];
    forks = s;
	
    sem_init(&dinnerQuotaLock, 0, num_phsp / 2);
    
  

	for (i = 0; i <= num_phsp; i++) {
		sem_init(&forks[i], 0, 1);
	}

	for (i = 0; i < num_phsp; ++i) {

        workerTask* arg = (workerTask * ) malloc(sizeof(workerTask));

        arg->dst = probDist;
        arg->max_dine = max_dine;
        arg->max_think = max_think;
        arg->min_dine = min_dine;
        arg->min_think = min_think;
        arg->numberOfDinners = numberOfDinners;
        arg->num_phsp = num_phsp;
        arg->i = i;
        pthread_create(&tid[i], NULL, &workerThread, (void *)arg);
    }

	pthread_exit(NULL);
}
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define RANDMAX 101

/* PROTOTYPES */

void *producer(void *param); 
void *consumer(void *param);

/* GLOBALS */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t buf_full;
sem_t buf_empty;
sem_t complete;

int times;
int con_count;
int num_consumed = 0;
int buffer[1];

/* producer */
void *produce (void *param)
{
	/* random number generated to produce */
	unsigned int seed = 55555;	
	int i = 0;
	while (i < times) {
		sem_wait(&buf_empty);	
		//produce (fill buffer)
		int num = rand_r(&seed) % RANDMAX;
		buffer[0] = num;
		//output
		printf("Producer: produced %d!\n", num);
		int q;
		for (q = 0; q < con_count; q++) {
			sem_post(&buf_full);
		}
		
		i++;
	}
	return NULL;
}

/* consumer */
void *consume (void *param)
{
	
	long thread_num = (long) param;
	int val;
	int i = 0;
	while (i < times) {
		sem_wait(&buf_full);

		pthread_mutex_lock(&mutex);
		//'read' the buffer if you are any thread
		val = buffer[0];
		num_consumed++;
		//output
		if (num_consumed == con_count) {
			printf("Consumer: I am thread %ld, and I have just read in the value: %d.\n---\n", thread_num, val);
			int k;
			for (k = 1; k < num_consumed; k++) {
				sem_post(&complete);
			}
			sem_post(&buf_empty);
			num_consumed = 0;
			pthread_mutex_unlock(&mutex);
		} else {
			printf("Consumer: I am thread %ld, and I have just read in the value: %d.\n", thread_num, val);
			pthread_mutex_unlock(&mutex);
			sem_wait(&complete);
		}
		i++;
	}
	return NULL;
}

/* MAIN */
int
main (int argc, char **argv)
{
	if (argc == 3) {
		times = atoi(argv[1]);
		con_count = atoi(argv[2]);
	} else {
		times = 4;
		con_count = 4;
	}
	//initialize buffer
	buffer[0] = 0;
	//initialize semaphores
	sem_init(&buf_empty, 0, 1);
	sem_init(&buf_full, 0, 0);
	sem_init(&complete, 0, 0);

	long i;

	//declare threads
	pthread_t producer_thread;
	pthread_t consumer_threads[con_count];

	printf("%d consumer threads, producing %d times.\n", con_count, times);

	//create les multiple consumer threads
	//(because we want to make this difficult)
	for (i = 0; i < con_count; i++) {
		(void) pthread_create(&consumer_threads[i], NULL, consume, (void*) i);
	}

	//create single producer thread
	(void) pthread_create(&producer_thread, NULL, produce, NULL);

	//join em back together
	(void) pthread_join(producer_thread, NULL);
	for (i = 0; i < con_count; i++) {
		(void) pthread_join(consumer_threads[i], NULL);
	}

	sem_destroy(&buf_empty);
	sem_destroy(&buf_full);
	sem_destroy(&complete);

	//all done
	printf("all done!\n");

	return 0;

}

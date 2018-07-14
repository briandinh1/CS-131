#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

/*Global variables */
int num_threads; 
pthread_mutex_t *mutexes;

const int COURSES = 3;

/* For representing the status of each philosopher */
typedef enum{
	none,   // No forks
	one,    // One fork
	two     // Both forks to consume
} utensil;

/* Representation of a philosopher */
typedef struct phil_data{
	int phil_num;
	int course;
	utensil forks; 
}phil_data;

/* ****************Change function below ***************** */
void *eat_meal( void* arg){
/* 3 course meal: Each need to acquire both forks 3 times.
 *  First try for fork in front.
 * Then for the one on the right, if not fetched, put the first one back.
 * If both acquired, eat one course.
 */
	phil_data *p = (phil_data*) arg;
	
	int frontFork = p->phil_num;
	int rightFork = (frontFork + 1) % num_threads;
	
	while(p->course < COURSES)
	{
		if (pthread_mutex_trylock(&mutexes[frontFork]) == 0)
		{
			if (pthread_mutex_trylock(&mutexes[rightFork]) == 0)
			{
				sleep(1);
			
				pthread_mutex_unlock(&mutexes[frontFork]);
				pthread_mutex_unlock(&mutexes[rightFork]);
				
				++p->course;
				printf("philosopher %d is finished with course %d\n", 
					p->phil_num + 1, p->course);	
			}
			else
			{
				pthread_mutex_unlock(&mutexes[frontFork]);
			}
		}
	}
	
	printf("***philosopher %d is done with all courses***\n", p->phil_num + 1);
}

/* ****************Add the support for pthreads in function below ***************** */
int main( int argc, char **argv ){
	int num_philosophers, error;

	if (argc < 2) {
          fprintf(stderr, "Format: %s <Number of philosophers>\n", argv[0]);
          return 0;
     }
    
    num_philosophers = num_threads = atoi(argv[1]);
	
	pthread_t threads[num_threads];
	phil_data philosophers[num_philosophers]; //Struct for each philosopher
	mutexes = malloc(sizeof(pthread_mutex_t)*num_philosophers); //Each mutex element represent a fork

	for (int i = 0; i < num_philosophers; ++i)
		pthread_mutex_init(&mutexes[i], NULL);
	
	for( int i = 0; i < num_philosophers; ++i )
	{
		philosophers[i].phil_num = i;
		philosophers[i].course   = 0;
		philosophers[i].forks    = none;
		pthread_create(&threads[i], NULL, &eat_meal, &philosophers[i]);
	}
	
	for (int i = 0; i < num_philosophers; ++i)
		pthread_join(threads[i], NULL);

	for(int i = 0; i < num_philosophers; ++i)
		pthread_mutex_destroy(&mutexes[i]);



	return 0;
}
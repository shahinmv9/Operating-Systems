/*
/-------------------\
| Shahin Mammadov   |
| Operating Systems |
| Assignment 4      |
\-------------------/
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

int persons = 100, n = 10000; //Default parameters
char coins[] = {'O','O','O','O','O','O','O','O','O','O','X','X','X','X','X','X','X','X','X','X'};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void run_threads(unsigned int nn, void* (*proc)(void *));//declaring the function foor run_threads

static const char *progname = "pthread";

//This code is from the assignment sheet
static double timeit(int n, void* (*proc)(void *))
{
    clock_t t1, t2;
    t1 = clock();
    run_threads(n, proc);
    t2 = clock();
    return ((double) t2 - (double) t1) / CLOCKS_PER_SEC * 1000;
}

//Modified the function given in the slides
void run_threads(unsigned int nn, void* (*proc)(void *))
{
    int i, rc;

    pthread_t *thread;
    thread = calloc(nn, sizeof(pthread_t));

    if (!thread) 
    {
        fprintf(stderr, "%s: %s: %s\n", progname, __func__, strerror(errno));
        exit(1);
    }

    for (i = 0; i < nn; i++) 
    {
        rc = pthread_create(&thread[i], NULL, proc, NULL); //creating thread for each person
        if (rc) 
        {
            fprintf(stderr, "pthread_create() failed: %s\n", strerror(rc));
        }
    }

    for (i = 0; i < nn; i++) 
    {
        if (thread[i]) 
        {
            (void) pthread_join(thread[i], NULL);
            if(rc)
            {
                fprintf(stderr, "pthread_join() failed: %s\n", strerror(rc));
            }
	    }
    }

    (void) free(thread);
}

char flip(char c)
{
    if(c == 'O')
    {
        return 'X'; //Returning side X
    }
    else
    {
        return 'O';//Returning side O
    }
}

static void *g_lock(void *data)
{
    int i, j;

    pthread_mutex_lock(&mutex); //we lock the mutex before each person obtains the coin
    for(i = 0; i < n; i++)
    {
        for(j = 0; j < 20; j++)
        {
            coins[j] = flip(coins[j]);
        }
    }
    pthread_mutex_unlock(&mutex);//unlock after we are done
    return NULL;
}

static void *i_lock(void *data)
{
    int i, j;

    for(i = 0; i < n; i++)
    {
        pthread_mutex_lock(&mutex); //we lock the mutex after each person obtains the coin
        for(j = 0; j < 20; j++)
        {
            coins[j] = flip(coins[j]);
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

static void *c_lock(void *data)
{
    int i, j;

    for(i = 0; i < n; i++)
    {
        for(j = 0; j < 20; j++)//we lock the mutex everytime person flips the coin
        {
            pthread_mutex_lock(&mutex);
            coins[j] = flip(coins[j]);
            pthread_mutex_unlock(&mutex);
        }
    }
    return NULL;
}

void run(char *coins, int persons, int n)
{
    double time_g = timeit(persons, g_lock); //Global Lock
    double time_i = timeit(persons, i_lock); //Iteration Lock
    double time_c = timeit(persons, c_lock); //Coin Lock

    printf("coins: %s (start - global lock)\n", coins);
    printf("coins: %s (end - global lock)\n", coins);
    printf("%d threads x %d flips: %.3lf ms\n\n", persons, n, time_g);

    printf("coins: %s (start - iteration lock)\n", coins);
    printf("coins: %s (end - table lock)\n", coins);
    printf("%d threads x %d flips: %.3lf ms\n\n", persons, n, time_i);

    printf("coins: %s (start - coin lock)\n", coins);
    printf("coins: %s (end - coin lock)\n", coins);
    printf("%d threads x %d flips: %.3lf ms\n\n", persons, n, time_c);
}

int main(int argc, char* argv[])
{
    int i;

    while((i = getopt(argc, argv, "p:n:")) != -1) //commandline input
    {
        if(i == 'p')
        {
            int persons_temp;
            persons_temp = atoi(optarg);
            if(persons_temp <= 0)
            {
                perror("Error, invalid number of people.\n");
                exit(1);
            }
            persons = persons_temp;
        }
        else if(i == 'n')
        {
            int n_temp;
            n_temp = atoi(optarg);
            if(n_temp <= 0)
            {
                perror("Error, invalid number of flips.\n");
                exit(1);
            }
            n = n_temp;
        }
        else
        {
            perror("Error, invalid command.\n");
            exit(1);
        }
    }
    
    run(coins, persons, n); //running all 3 strategies in this function

    return 0;
}

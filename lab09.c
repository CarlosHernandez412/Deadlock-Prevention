#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      /* header file for the POSIX API */
#include <string.h>      /* string handling functions */
#include <errno.h>       /* for perror() call */
#include <pthread.h>     /* POSIX threads */ 
#include <sys/ipc.h>     /* SysV IPC header */
#include <sys/sem.h>     /* SysV semaphore header */
#include <sys/syscall.h> /* Make a syscall() to retrieve our TID */

#define NUM_THREADS 5

/* global variables */
int LIMIT = 50;

int fib(int n) {
    if (n < 2) return n;
    return fib(n-1) + fib(n-2); 
}

/* Structure to pass information to thread function */
typedef struct {
    int philID;    
    int fork1;   
    int fork2;     
} t_info;

/* Union to hold semaphore information. Call it 'my_semun' in program */
union semun {
    int  val;
   // struct semid_ds *buf;
   // unsigned short  *array;
    //struct seminfo  *__buf;
} my_semun;


int semid,ret;

struct sembuf grab_fork[5][2];
struct sembuf release_fork[5][1];

pthread_t threads[NUM_THREADS];
t_info data[NUM_THREADS];

void *doubleIt(void *);

int main(int argc, char *argv[])
{
    char pathname[256];
    getcwd(pathname, 200);
    strcat(pathname, "/foo");
   
	int i; 
    int nsem = 5;
    key_t ipckey = ftok(pathname, 11);
    semid = semget(ipckey, nsem, 0666 | IPC_CREAT);


    if(argc > 1) {
        LIMIT = atoi(argv[1]);
        printf("Limit set to %d\n", LIMIT);
    }
    else {
        printf("Limit set to %d\n", LIMIT);
    }
    
    printf("Created semaphore with ID: %d\n", semid);
    

  /*  printf("Philosopher %d will pick up fork %d, then fork %d\n", data[i].philID);
    printf("Philosopher %d will pick up fork %d, then fork %d\n", data[i].philID);
    printf("Philosopher %d will pick up fork %d, then fork %d\n", data[i].philID);
    printf("Philosopher %d will pick up fork %d, then fork %d\n", data[i].philID);
    printf("Philosopher %d will pick up fork %d, then fork %d\n", data[i].philID);
*/
    for( int i =0; i < 5; i++) {
		data[i].philID = i;
         //i is odd 
        if(i % 2 == 1) {
           data[i].fork1 = i;
            data[i].fork2 =( i + 1) % 5;
			printf("Philosopher %d will pick up fork %d, then fork %d\n", data[i].philID, data[i].fork1, data[i].fork2);
        }
        // if the process is even 
        else if(i % 2 == 0) {
            data[i].fork1 = (i + 1) % 5;
            data[i].fork2 = i;
			printf("Philosopher %d will pick up fork %d, then fork %d\n", data[i].fork2, data[i].fork1, data[i].fork2);
        }
    } 
	printf("Initializing variables....\n");

    my_semun.val = 0;
	semctl(semid, 0, SETVAL, my_semun);
	
	for(i = 0; i < 5; i++)
	{
		grab_fork[i][0].sem_num = i;
		grab_fork[i][0].sem_op = 0;
		grab_fork[i][0].sem_flg = SEM_UNDO;
		grab_fork[i][1].sem_num = i;
		grab_fork[i][1].sem_op = +1;
		grab_fork[i][1].sem_flg = SEM_UNDO;
		release_fork[i][0].sem_num = i;
		release_fork[i][0].sem_op = -1;
		release_fork[i][0].sem_flg = SEM_UNDO;
	}
   

   for(i = 0; i < 5; i++) 
	{
		printf("Spawning thread for philosopher %d...\n", i);
        ret == pthread_create(&threads[i], NULL, doubleIt, (void*) &(data[i]));
        if(ret) 
		{
            perror("pthread_create: ");
            exit(EXIT_FAILURE);
        }
    }

    /* Calling pthread_join for all philospher threads */
    for(i = 0; i < 5; i++) {
        printf("Calling pthread_join for Philosopher %d...\n", i);
        if(pthread_join(threads[i], NULL) < 0) 
		{
            perror("pthread_join: ");
        }
		printf("Philosopher %d has finished\n", data[i].philID);
    }
    /* Delete the semaphore with semctl */
    printf("Deleting semaphores with ID: %d\n", semid);
    ret = semctl(semid, 0, IPC_RMID);
    exit(EXIT_SUCCESS);
}

void *doubleIt(void *info)
{
	t_info *data;
	int ret;

	data = (t_info *)info;
	int i;
    pid_t tid = syscall(SYS_gettid);
    fprintf(stdout,"Philospher %d thread pid: %d tid: %d \n", data->philID, getpid(), tid);

    for( i = 0; i < 5; i++) 
    {
        printf("Philosopher %d is thinking...\n", data->philID);
        fib(40);
        printf("Philosopher %d is hungry...\n", data->philID);
        ret = semop(semid, grab_fork[data[i].fork1], 2);
        ret = semop(semid, grab_fork[data[i].fork2], 2);
        printf("Philosopher %d is eating.\n", data->philID);
        fib(30);
        ret = semop(semid, release_fork[data[i].fork2], 1);
        ret = semop(semid, release_fork[data[i].fork1], 1);
        printf("Philosopher %d has put down both forks.\n", data->philID);
    }
        pthread_exit(0);
}


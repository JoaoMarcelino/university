//compile with: gcc -Wall stockmarket.c semlib.c -o stock
//using SysV semaphores
//Note: you can change all the operations of semaphores to make it work with POSIX semaphores

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h> 

#define NUM_WRITERS 5		// number of writers
#define NUM_READERS 5		// number of readers
#define STOCKLIST_SIZE 10  // stock list slots
#define MUTEX 0
#define STOP_WRITERS 1

typedef struct
{
	int readers;
	int slots[STOCKLIST_SIZE];
} mem_structure;

int semid, shmid;
pid_t childs[NUM_READERS + NUM_WRITERS];
mem_structure *stocklist;
sem_t mutex,stop_writers;

void cleanup(int signo) // clean up resources by pressing Ctrl-C
{
	int i = 0;
	while (i < (NUM_READERS + NUM_WRITERS))
		kill(childs[i++], SIGKILL);
	while (wait(NULL) != -1);

	if (semid >= 0){	// remove semaphores
		sem_close(&mutex);
		sem_close(&stop_writers);
	}
	if (shmid >= 0) // remove shared memory
		shmctl(shmid, IPC_RMID, NULL);


	printf("Closing...\n");
	exit(0);
}


int get_stock_value()
{
	return 1 + (int) (100.0 * rand() / (RAND_MAX + 1.0));
}

int get_stock()
{
	return (int) (rand() % STOCKLIST_SIZE);
}

void write_stock(int n_writer, mem_structure *stocklist)
{
	int stock=get_stock();
	int stock_value=get_stock_value();
	stocklist->slots[stock] = stock_value;
	fprintf(stderr, "Stock %d updated by BROKER %d to %d\n", stock, n_writer, stock_value);
}

int writer_code(int n_writer)
{
	srand(getpid());

	while (1)
	{
		/*insert code here*/
		write_stock(n_writer,stocklist);
		sleep(1 + (int) (10.0 * rand() / (RAND_MAX + 1.0)));
	}

	exit(0);
}

void read_stock(int pos, int n_reader, mem_structure *stocklist)
{
		fprintf(stderr, "Stock %d read by client %d = %d\n", pos, n_reader, stocklist->slots[pos]);
}

int reader_code(int n_reader)
{
	srand(getpid());	// to obtain a different seed

	while (1)
	{
		/*insert code here*/
		read_stock(get_stock(),n_reader,stocklist);
		sleep(1 + (int) (3.0 * rand() / (RAND_MAX + 1.0)));
	}

	exit(0);
}

void monitor() // main process monitors the reception of Ctrl-C
	{
	struct sigaction act; 
	act.sa_handler = cleanup; 
	act.sa_flags = 0; 
	if ((sigemptyset(&act.sa_mask) == -1) || 
		(sigaction(SIGINT, &act, NULL) == -1)) 
		perror("Failed to set SIGINT to handle Ctrl-C");
	while(1){
		sleep(5);
		printf("Still working...\n");
		}
	exit(0);
	}
-

int main()
{
	int i = 0;
	int j = 0;
	sem_init(&mutex,0,1);
	sem_init(&stop_writers,0,1);
	// Create shared memory
	if ((shmid= shmget(IPC_PRIVATE,sizeof(int),IPC_CREAT| 0700))==-1){
		perror("Error in shamget with IPC_CREAT\n");
		exit(1);
	}
	
	// Attach shared memory
	if((stocklist=shmat(shmid,NULL,0))<=0){
		perror("Error in shmat\n");
		exit(1);
	}
	

	stocklist->readers = 0;

	while (i < NUM_WRITERS)
	{
		if ((childs[j] = fork()) == 0)
		{
			sem_wait(&stop_writers);
			sem_wait(&mutex);
			writer_code(i);
			sem_post(&stop_writers);
			sem_post(&mutex);
			exit(0);
		}
		else if (childs[j] == -1)
		{
			perror("Failed to create reader process");
			exit(1);
		}
		++i;
		++j;
	}
	i = 0;
	while (i < NUM_READERS)
	{
		if ((childs[j] = fork()) == 0) {

			sem_wait(&mutex);
			stocklist->readers++;
			if(stocklist->readers==1)
				sem_wait(&stop_writers);
			sem_post(&mutex);


			reader_code(i);

			sem_wait(&mutex);
			stocklist->readers--;
			sem_post(&stop_writers);
			sem_post(&mutex);
			exit(0);
		}
		else if (childs[j] == -1) {
			perror("Failed to create writer process");
			exit(1);
		}
		++i;
		++j;
	}
	monitor();
	exit(0);
}


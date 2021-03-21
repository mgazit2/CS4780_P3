/* Matan Gazit
 * CS 4760
 * 03/17/21
 * Assignment 3
 * child.c
 *
 * child executable for program
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#define TIME_SIZE 50

enum type {CON, PROD}; // 0 or 1, effectively
enum res {FOOD, NO_FOOD};

/*PROTOTYPE*/
void term_handler(int sig);
void get_time();

/*SHARED*/
int sem_empty_key;
int sem_empty_id;
sem_t* sem_empty;

int sem_full_key;
int sem_full_id;
sem_t* sem_full;

int mutex_key;
int mutex_id;
pthread_mutex_t* mutex;

int buffer_key;
int buffer_id;
int* buffer;

int in_key;
int in_id;
int* in;

int out_key;
int out_id;
int* out;

int group_pid_id;
int group_pid_key;
pid_t* group_pid;

int filename_id;
int filename_key;
char* filename;

int curr_prod_id;
int curr_prod_key;
int* curr_prod;

int curr_con_id;
int curr_con_key;
int* curr_con;

/*GLOBALS*/
int state;
char curr_time[TIME_SIZE];
FILE* file;

int main(int argc, char* argv[])
{
	signal(SIGTERM, term_handler);
	signal(SIGINT, term_handler);

	setvbuf(stdout, NULL, _IONBF, 0); // for debugging purposes
	//FILE* file;

	if (argc == 1)
	{
		printf("Child executed without state flag... exiting...\n");
		exit(1);
	}
	else
	{
		state = atoi(argv[1]);
		printf("State: %d\n", state);
	}

	/*Shared Memory ftok()*/
	sem_empty_key = ftok("makefile", 1);
	sem_full_key = ftok("makefile", 2);
	mutex_key = ftok("makefile", 3);
	buffer_key = ftok("makefile", 4);
 	in_key = ftok("makefile", 5);
	out_key = ftok("makefile", 6);
	group_pid_key = ftok("makefile", 7);
	filename_key = ftok("makefile", 8);
	curr_prod_key = ftok("makefile", 9);
	curr_con_key = ftok("makefile", 10);

	/*Shared Memory Allocation Checks*/
	// sem empty
	if ((sem_empty_id = shmget(sem_empty_key, sizeof(sem_t), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for sem_empty\n");
		exit(1);
	}
	else
	{
		sem_empty = (sem_t *)shmat(sem_empty_id, NULL, 0);
	}
	// sem full
	if ((sem_full_id = shmget(sem_full_key, sizeof(sem_t), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for sem_full\n");
		exit(1);
	}
	else
	{
		sem_full = (sem_t *)shmat(sem_full_id, NULL, 0);
	}
	// mutex
	if ((mutex_id = shmget(mutex_key, sizeof(pthread_mutex_t), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0) 
	{   
		perror("ERROR: Shmget failed to allocate memory for sem_full\n");
		exit(1);
	}
	else
	{
		mutex = (pthread_mutex_t *)shmat(mutex_id, NULL, 0);
	}
	// buffer
	if ((buffer_id = shmget(buffer_key, sizeof(int) * 5, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for buffer array\n");
		exit(1);
	}
	else
	{
		buffer = (int *)shmat(buffer_id, NULL, 0);
	}
	// in
	if ((in_id = shmget(in_key, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{     
		perror("ERROR: Shmget failed to allocate memory for in flag\n");
		exit(1);
	}
	else
	{
		in = (int *)shmat(in_id, NULL, 0);
	}
	// out
	if ((out_id = shmget(out_key, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for out\n");
		exit(1);
	}
	else
	{
		out = (int *)shmat(out_id, NULL, 0);
	}
	// group pid
	if ((group_pid_id = shmget(group_pid_key, sizeof(pid_t), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for group_pid\n");
		exit(1);
	}
	else
	{
		group_pid = (pid_t *)shmat(group_pid_id, NULL, 0);
	}
	// filename
	if ((filename_id = shmget(filename_key, sizeof(char) * 26, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for filename\n");
		exit(1);
	}
	else
	{
		filename = (char *)shmat(filename_id, NULL, 0);
		file = fopen(filename, "a");
	}
	// curr prod
	if ((curr_prod_id = shmget(curr_prod_key, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for curr prod integer\n");
		exit(1);
	}
	else
	{
		curr_prod = (int *)shmat(curr_prod_id, NULL, 0);
	}
	// curr con
	if ((curr_con_id = shmget(curr_con_key, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for curr con integer\n");
		exit(1);
	}
	else
	{
		curr_con = (int *)shmat(curr_con_id, NULL, 0);
	}


	if (state == CON)
	{
		printf("I am a consumer\n"); // states its process type
		sleep((rand() % 10) + 1); // sleep between 1 and 10 seconds
		printf("Awake and waiting on sem_full\n");
		sem_wait(&*sem_full);
		printf("Received sem_full\n");
		pthread_mutex_lock(&*mutex);
		//printf("Waiting on mutex\n");
		int item = buffer[*out];
		get_time();
		fprintf(file, "\nITEM USED BY A PROCESS AT %s WITH PID %d AND STATE %d\n", curr_time, getpid(), state);
		if (item != FOOD) {
			printf("A process has starved!\n");
			fprintf(file, "\nPROCESS STARVED AT %s WITH PID %d AND STATE %d\n", curr_time, getpid(), state);
			fclose(file);
			exit(1);
		}
		buffer[*out] = NO_FOOD;
		//*out++;
		//*out = *out % 5;
		pthread_mutex_unlock(&*mutex);
		sem_post(&*sem_empty);
		printf("I am fed and exiting succesfully!\n");
		(*curr_con)--;
		fclose(file);
		exit(0);
	}
	else if (state == PROD)
	{
		printf("I am a producer\n");
		sleep((rand() % 5) + 1); // sleep between 1 and 5 seconds
		printf("Awake and waiting on sem_empty\n");
		int item = FOOD;
		get_time();
		fprintf(file, "\nITEM USED BY A PROCESS AT %s WITH PID %d AND STATE %d\n", curr_time, getpid(), state);
		sem_wait(&*sem_empty);
		printf("Received sem_empty\n");
		pthread_mutex_lock(&*mutex);
		buffer[*in] = item;
		//*in++;
		//*in = *in % 5;
		pthread_mutex_unlock(&*mutex);
		sem_post(&*sem_full);
		printf("I have finished producing succesfully\n");
		(*curr_prod)--;
		fclose(file);
		exit(0);
	}
	else
	{
		printf("I am nothing... How did I get here?\n");
		fclose(file);
		exit(1);
	}
	
	fclose(file);	
	return EXIT_SUCCESS;
}

void term_handler(int sig)
{
	perror("Child: Received termination signal from parent... Exiting...\n");
	get_time();
	fprintf(file, "\nCHILD TERMINATED AT TIME %s WITH PID %d AND STATE %d\n", curr_time, getpid(), state);
	fclose(file);
	exit(1);
}

void get_time()
{
	time_t curr;
	struct tm *loc_time;

	curr = time(NULL);

	loc_time = localtime(&curr);

	strftime(curr_time, TIME_SIZE, "%H:%M:%S\n", loc_time);
}

/* Matan Gazit
 * CS 4760
 * 03/17/21
 * main.c
 *
 * main executable of assignment 3
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>

#define DEF_PROD 2 // default # producers (processes)
#define DEF_CON 6 // default # consumers (processes)
#define DEF_RUNTIME 100 // default program runtime before termination (seconds)
#define MAX_PROCS 20 // maximum number of processes allowed to program
#define TIME_SIZE 50 // for time string

/*PROTOTYPES*/
void print_usage();
void deallocate();
void spawn_pc(int id);
void timeout(int sig);
void kill_all(int sig);
void get_time();

/*GLOBALS*/
static char* _name; // program name
int num_prod; // stores # of producers
int num_cons; // stores # of consumers
int runtime; // stores maximum program runtime
FILE* file;
char curr_time[50];
int count;

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

int main (int argc, char* argv[])
{
	// initial calls
	signal(SIGINT, kill_all);
	signal(SIGALRM, timeout);
	setvbuf(stdout, NULL, _IONBF, 0); // for debugging purposes
	
	// initial initializations
	num_prod = DEF_PROD;
	num_cons = DEF_CON;
	runtime = DEF_RUNTIME;
	_name = argv[0];
	char* file_out = "logfile"; // shared output filename for program children

	// exits before shared memory allocation if no arguments found
	if (argc == 1)
	{
		printf("Run program with -h for usage options...\n");
		return EXIT_SUCCESS;
	}
	
	// getopt loop
	while (true)
	{
		int c = getopt(argc, argv, "ho:p:c:t:");
		if (c == -1) break;
		switch (c)
		{
			case 'h':
				print_usage();
				return EXIT_SUCCESS;
				break;

			case 'o':
				file_out = optarg;
				printf("Using custom filename for [logfile]: %s\n", file_out);
				break;

			case 'p':
				num_prod = atoi(optarg);
				printf("Using custom max number of producers: %d\n", num_prod);
				break;

			case 'c':
				num_cons = atoi(optarg);
				printf("Using custom max number of consumers: %d\n", num_cons);
				break;

			case 't':
				runtime = atoi(optarg);
				printf("Using custom max program runtime: %d\n", runtime);
				break;

			default:
				printf("Unknown input read... Exiting program...\n");
				return EXIT_FAILURE;
				break;
		} // end switch getopt
	} // end while getopt

	// check sum of prod and con <= 20
	if (num_prod + num_cons > MAX_PROCS)
	{
		printf("Sum of chosen values for maximum number of producers and consumers exceeds %d\n", MAX_PROCS);
		num_prod = DEF_PROD;
		num_cons = DEF_CON;
		printf("Variables were set to the default value noted in the README for this run\n");
	}

	/*Shared Memory ftok()'s*/
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
		//sem_post(&*sem_empty);
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
		perror("ERROR: Shmget failed to allocate memory for mutex\n");
		exit(1);
	}   
	else   
	{     
		mutex = (pthread_mutex_t *)shmat(mutex_id, NULL, 0);   
	}
	// buffer
	if ((buffer_id = shmget(buffer_key, sizeof(int) * 5, IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{     
		perror("ERROR: Shmget failed to allocate memory for food buffer\n");
		exit(1);
	}   
	else
	{     
		buffer = (int *)shmat(buffer_id, NULL, 0);
		int i = 0;
		for (; i < 5; i++) buffer[i] = 0;
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
		*in = 0;
	}
	// out
	if ((out_id = shmget(out_key, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{     
		perror("ERROR: Shmget failed to allocate memory for out flag\n");
		exit(1);
	}   
	else
	{     
		out = (int *)shmat(out_id, NULL, 0);
		*out = 0;
	}
	// group pid
	if ((group_pid_id = shmget(group_pid_key, sizeof(pid_t), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for group pid\n");
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
		strcpy(filename, file_out);
		file = fopen(filename, "a");
	}
	// curr prod
	if ((curr_prod_id = shmget(curr_prod_key, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for curr_prod integer\n");
		exit(1);
	}
	else
	{
		curr_prod = (int *)shmat(curr_prod_id, NULL, 0);
		*curr_prod = 0;
	}
	// curr cons
	if ((curr_con_id = shmget(curr_con_key, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR)) < 0)
	{
		perror("ERROR: Shmget failed to allocate memory for curr_con integer\n");
		exit(1);
	}
	else
	{
		curr_con = (int *)shmat(curr_con_id, NULL, 0);
		*curr_con = 0;
	}

	pthread_mutex_init(&*mutex, NULL); // init mutex
	sem_init(&*sem_empty, 1, 0); // init empty
	sem_init(&*sem_full, 1, 0); // init full
	sem_post(&*sem_empty); // starts the semaphore cycle

	// set counters
	int i = 0;
	count = 1; // main is 1st process in monitor exec
	
	alarm(runtime); // set alarm

	printf("\nProgram will run for %d seconds\n", runtime);
	printf("Program is allowed %d producers\n", num_prod);
	printf("Program is allowed %d consumers\n", num_cons);

	// loop produces producers
	
	// the contents of these loops can be encapsulated for further readability
	while (i < num_prod)
	{
		int id;
		id = 1;
		spawn_pc(id);
		i++;
		(*curr_prod)++;
		count++;
		sleep(1); // for log readability
	}
	i = 0;

	// loop produces consumers
	while (i < num_cons)
	{
		int id;
		id = 0;
		spawn_pc(id);
		i++;
		(*curr_con)++;
		count++;
		sleep(1); // for log readability
	}
	i = 0;

	// loop keeps 'monitor' exec alive while 'child' execs work
	while (*curr_prod != 0 && *curr_con != 0)
	{
		if (*curr_prod == *curr_con)
		{
			printf("Equal...\n");
			continue;
		}
		else if (*curr_prod > *curr_con)
		{
			int id;
			id = 0;
			spawn_pc(id);
			i++;
			(*curr_con)++;
			count++;
			sleep(1); // for log readability
			continue;
		}	
		else
		{
		  int id;
			id = 1;
			spawn_pc(id);
			i++;
			(*curr_prod)++;
			count++;
			sleep(1); // for log readability
			continue;
		}

		if (*curr_con > 0)
		{
			printf("In this weird loop area...\n");
			int id;       
			id = 1;
			spawn_pc(id);
			i++;
			(*curr_prod)++;
			count++;
			sleep(1); // for log readability
			continue;
		}
	}

	// final checks before terminating
	printf("%d %d", *curr_prod, *curr_con);
	sleep(10);
	get_time();
	fprintf(file, "\nPARENT PROCESS LOGGING SUCCESFUL EXECUTION FINISHING AT TIME: %s\n", curr_time);
	fclose(file);
	deallocate();
	printf("Back in main\n");
	return EXIT_SUCCESS;
}

// prints program usage
void print_usage()
{
	printf("monitor [-h] [-o logfile] [-p m] [-c n] [-t time]\n");
	printf("See README for usage descriptions\n");
	printf("[-h] Describe how the project should be run and then terminate.\n");
	printf("[-o logfile] Name of the file to save logs; default: logfile\n");
	printf("[-p m] Number of producers; default: m = 2\n");
	printf("[-c n] Number of consumers; default: n = 6\n");
	printf("[-t time] The time in seconds after which the process will terminate, even if it has not finished. default: time = 100\n");
} // end print_usage()

// marks all shared memory segments for deletion
// does not force delete all memory segments
// memory segments will remain in system if processes are still running
// see ipcs(1) & ipcrm(1) for details
void deallocate()
{
	shmdt(sem_empty);
	shmctl(sem_empty_id, IPC_RMID, NULL);

	shmdt(sem_full);
	shmctl(sem_full_id, IPC_RMID, NULL);

	shmdt(mutex);
	shmctl(mutex_id, IPC_RMID, NULL);

	shmdt(buffer);
	shmctl(buffer_id, IPC_RMID, NULL);

	shmdt(in);
	shmctl(in_id, IPC_RMID, NULL);

	shmdt(out);
	shmctl(out_id, IPC_RMID, NULL);

	shmdt(group_pid);
	shmctl(group_pid_id, IPC_RMID, NULL);

	shmdt(filename);
	shmctl(filename_id, IPC_RMID, NULL);

	shmdt(curr_prod);
	shmctl(curr_prod_id, IPC_RMID, NULL);

	shmdt(curr_con);
	shmctl(curr_con_id, IPC_RMID, NULL);

	sem_destroy(&*sem_full);
	sem_destroy(&*sem_empty);

	printf("Shared memory succesfully deallocated...\n");
}

// spawns a producer/consumer
void spawn_pc(int id)
{
	char d[10];
	sprintf(d, "%d", id);
	printf("Spawning worker with id: %d\n", id);
	if (fork() == 0)
	{
		if (count == 1)
		{
			(*group_pid) = getpid();
		}
		setpgid(0, (*group_pid));
		if (id == 0) execl("./consumer", "consumer", d, NULL);
		else if (id == 1) execl("./producer", "producer", d, NULL);
		//execl("./child", "child", d, NULL);
		exit(0);
	}
}

// signal handler for alarm
void timeout(int sig)
{
	perror("timeout()");
	int i = 0;
	printf("We're out of time, folks...\n");
	killpg((*group_pid), SIGTERM);
	for (; i < 20; i++) wait(NULL);
	printf("Exited all active processes...\n");
	get_time();
	fprintf(file, "\nPARENT PROCESS LOGGING TIMEOUT TERMINATION! EXECUTION FINISHING AT TIME: %s\n", curr_time);
	fclose(file);
	deallocate();
	exit(1);
}

// signal handler for i/o interrupt
void kill_all(int sig)
{
	perror("kill_all()");
	int i = 0;
	printf("Received termination signal, exiting...\n");
	killpg((*group_pid), SIGTERM);
	for (; i < 20; i++) wait(NULL);
	printf("Exited all active processes... \n");
	get_time();
	fprintf(file, "\nPARENT PROCESS LOGGING I/O TERMINATION! EXECUTION FINISHING AT TIME: %s\n", curr_time);
	fclose(file);
	deallocate();
	exit(1);
}

// bad time function
void get_time() 
{   
	time_t curr;
	struct tm *loc_time;
	curr = time(NULL);
	loc_time = localtime(&curr);
	strftime(curr_time, TIME_SIZE, "%H:%M:%S\n", loc_time);
}

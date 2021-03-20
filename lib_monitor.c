/* Matan Gazit
 * CS 4760
 * 03/20/2021
 * Assignment 3
 * lib_monitor.c
 *
 * static library for monitor function
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

void exec_con(int gpid)
{
	if (fork() == 0)
	{
		printf("Monitor forking consumer...\n");
		setpgid(0, gpid);
		execl("./child", "child", "0", NULL);
		exit(0);
	}
}

void exec_prod(int gpid)
{
	if (fork() == 0)
	{
		printf("Monitor forking producer...\n");
		setpgid(0, gpid);
		execl("./child", "child", "1", NULL);
		exit(0);
	}
}

void check_state(int gpid, int curr_prods, int curr_cons)
{
	if (curr_prods < curr_cons)
	{
		exec_prod(gpid);
	}
	else if (curr_prods > curr_cons)
	{
		exec_con(gpid);
	}
	else
	{
		printf("No need to spawn anymore producers or consumers at this time...");
	}
}


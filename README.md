Matan Gazit
CS 4760
Assignment 3

Purpose:

	The goal of this homework is to become familiar with semaphores in Linux.
	In addition, you will implement the conceptof a monitor, provide it as a statically linked library, and use it to solve the problem.
	Since you do not have condition variables, you will have to implement those using semaphores.  
	You will work on solving the Producer-Consumer problemby using the monitor.

Task:
		
	Implement a monitor using semaphores and shared memory and use it to solve the producer-consumer problem.
	You should have m producers and n consumers.  Your main process, called monitor, will fork all these producers and consumers.
	Use the template for monitor from the dining philosophers problem as discussed in lectures. 
	Your producers will produce an item and sleep for a random amount of time (between 1 and 5 seconds).  
	When the item is produced, it will be logged with the time and producer number in a log file.
	The consumers will sleep for random amountof time (between 1 and 10 seconds) and pick up the item and consume it.
	Consumers will also log their activity into the log file. Use the same log file for both producers and consumers.
	Make sure that you have more consumers than producers. In addition, the main process should print a message when an interrupt signal (^C) is received.
	All the children should bekilled as a result.
	All other signals are ignored.  Make sure that the processes handle multiple interrupts correctly.
	As aprecaution, add this feature only after your program is well debugged.
	The code for main and child processes should be compiled separately and the executables be called monitor, producer, and consumer.
	Make sure that you do not use absolute path for the child processes.
	Other points to remember:  You are required to use fork, exec (or one of its variants), wait, and exit to manage multiple processes. 
	Use shmctl suite of calls for shared memory allocation.  
	Also make sure that you do not have more than twenty processes in the system at any time.
	You can do this by keeping a counter in monitor that gets incremented by fork and decremented by wait.

Invoke:

	monitor [-h] [-o logfile] [-p m] [-c n] [-t time]
	
	-h Describe how the project should be run and then, terminate.
	-o logfile Name of the file to save logs; default:logfile
	-p m Number of producers; default:m= 2
	-c n Number of consumers; default:n= 6
	-t time The time in seconds after which the process will terminate, even if it has not finished; default:100

Make:

	While in the program directory, type 'make' into the command line to compile the project

Clean:

	While in the program directory, type 'make clean' into the command line to clean the project directory before recompilation

Issues:

	My primary issue with this project was utilizing a static library to implement the monitor functions.
		This was not an issue when handling the semaphore and mutex variables in the executable code, but led to
		error when the static library was called.

		The library was able to be compiled and called, but I believe the shared memory variables themselves were not
			correctly implemented by myself. Outside of this, there were no issues with implementing monitor functions
			into this assignment utilizing the semaphore and pthread libraries
	
	BUG: I have encountered a bug in my program where sometimes it gets stuck in an infinite loop in the produce/consume stage.
				Luckily, the termination commands work well and kill all processes if this occurs (timeout or i/o interrupt)

References:

	This project is heavily built on my Assignment 2 from this class, this semester

	Documentation used:

		https://man7.org/linux/man-pages/man3/sem_init.3.html
		https://man7.org/linux/man-pages/man3/sem_destroy.3.html
		https://man7.org/linux/man-pages/man3/sem_post.3.html
		https://man7.org/linux/man-pages/man3/sem_wait.3.html
		https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html
		https://man7.org/linux/man-pages/man3/pthread_mutex_destroy.3p.html

	Examples used for various implementations featured in this project

		https://www.geeksforgeeks.org/static-vs-dynamic-libraries/

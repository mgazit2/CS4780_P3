/* Matan Gazit
 * CS 4760
 * 03/20/2021
 * Assignment 3
 *
 * lib_monitor.h
 */

void exec_prod(int gpid);
void exec_con(int gpid);
void check_state(int gpid, int curr_prods, int curr_cons);
void consume_wait(sem_t empty, sem_t full, pthread_mutex_t mutex);
void consume_done(sem_t empty, sem_t full, pthread_mutex_t mutex);
void produce_wait(sem_t empty, sem_t full, pthread_mutex_t mutex);
void produce_done(sem_t empty, sem_t full, pthread_mutex_t mutex);

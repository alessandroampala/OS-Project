#include "my_sem_lib.h"

#include <errno.h>
#include <unistd.h>


#define TEST_ERROR if (errno) {fprintf(stderr,				\
				       "%s:%d: PID=%5d: Error %d (%s)\n", \
				       __FILE__,			\
				       __LINE__,			\
				       getpid(),			\
				       errno,				\
				       strerror(errno));}

/* Set a semaphore to a user defined value */
int sem_set_val(int sem_id, int sem_num, int sem_val) {

	return semctl(sem_id, sem_num, SETVAL, sem_val);
}

/* Try to access the resource */
int sem_reserve(int sem_id, int sem_num) {
	struct sembuf sops;
	
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semop(sem_id, &sops, 1);
}

/* Try to access the resource, if not available wait for the time specified in timeout */
int sem_reserve_timed(int sem_id, int sem_num, const struct timespec *timeout) {
	struct sembuf sops;
	
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = 0;
	return semtimedop(sem_id, &sops, 1, timeout);
}

/* Try to access the resource, don't block if not available */
int sem_reserve_nonblocking(int sem_id, int sem_num) {
	struct sembuf sops;
	
	sops.sem_num = sem_num;
	sops.sem_op = -1;
	sops.sem_flg = IPC_NOWAIT;
	return semop(sem_id, &sops, 1);
}

/* Release the resource */
int sem_release(int sem_id, int sem_num) {
	struct sembuf sops;
  
	sops.sem_num = sem_num;
	sops.sem_op = 1;
	sops.sem_flg = 0;
	
	return semop(sem_id, &sops, 1);
}

/* Release many resources */
int sem_release_many(int sem_id, int sem_num, unsigned int n_releases) {
	struct sembuf sops;
  
	sops.sem_num = sem_num;
	sops.sem_op = n_releases;
	sops.sem_flg = 0;
	
	return semop(sem_id, &sops, 1);
}

/* Wait for a semaphore to be zero */
int sem_wait_for_zero(int sem_id, int sem_num)
{
	struct sembuf sops;
  
	sops.sem_num = sem_num;
	sops.sem_op = 0;
	sops.sem_flg = 0;

	return semop(sem_id, &sops, 1);
}

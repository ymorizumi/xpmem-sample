#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <xpmem.h>

#define MAP_SIZE (2*1024*1024)
#define TEST_LOOP (32)
#define IS_SHMEM_TEST(i) ((i) < (TEST_LOOP>>1))

struct shmctrl {
	sem_t sem1; 		/* wait receiver, post main */
	sem_t sem2; 		/* wait main, post receiver */
	xpmem_segid_t segid;	/* xpmem segid */
	long pattern;		/* data fill pattern */
	struct timespec t;	/* start time */
};

static void *create_shmem (const char *name, int size)
{
	int fd;
	void *ptr;
	fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
	ftruncate(fd, size);
	ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	return ptr;
}

static void receiver (struct shmctrl *ctrl, long *share, long *local)
{
	struct xpmem_addr addr;
	int i, p;
	long *xpmem, delta;
	struct timespec t;

	sem_wait(&ctrl->sem1);	/* wait xpmem_make */
	addr.apid = xpmem_get(ctrl->segid, XPMEM_RDWR, XPMEM_PERMIT_MODE, NULL);
	addr.offset = 0;
	xpmem = (long *)xpmem_attach(addr, MAP_SIZE, NULL);
	printf("apid=%llx, xpmem=%p\n", addr.apid, xpmem);
	sem_post(&ctrl->sem2);	/* end xpmem_get.attach */

	for (i = 0; i < TEST_LOOP; i++) {
		sem_wait(&ctrl->sem1);	/* wait start share memory */
		memcpy(local, (IS_SHMEM_TEST(i) ? share : xpmem), MAP_SIZE);
		clock_gettime(CLOCK_MONOTONIC, &t);
		delta = (t.tv_sec - ctrl->t.tv_sec) * 1000 * 1000;
		delta += (t.tv_nsec - ctrl->t.tv_nsec) / 1000;
		printf("type=%s, time=%ld us\n", IS_SHMEM_TEST(i) ? "shmem" : "xpmem", delta);
		for (p = 0; p < MAP_SIZE / sizeof(long); p++) {
			if (local[p] != ctrl->pattern) {
				printf("verify faild\n");
				break;
			}
		}
		sem_post(&ctrl->sem2);
	}
	xpmem_detach(xpmem);
	xpmem_release(addr.apid);
	munmap(local, MAP_SIZE);
	munmap(share, MAP_SIZE);
	munmap(ctrl, sizeof(struct shmctrl));
}

int main (void)
{
	pid_t pid;
	int status, i, p;
	long *share, *local;
	struct shmctrl *ctrl;

	printf("xpmem_version=%x, buffer=%d byte\n", xpmem_version(), MAP_SIZE);
	ctrl = (struct shmctrl *)create_shmem("/xpmem_ctrl", sizeof(struct shmctrl));
	share = (long *)create_shmem("/xpmem_share", MAP_SIZE);
	local = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANONYMOUS, 0 , 0);
	sem_init(&ctrl->sem1, 1, 0);
	sem_init(&ctrl->sem2, 1, 0);

	if ((pid = fork()) == 0) {
		receiver(ctrl, share, local);
		return 0;
	}
	ctrl->segid = xpmem_make(local, MAP_SIZE, XPMEM_PERMIT_MODE, (void *)0666);
	printf("segid=%llx, ctrl=%p, local=%p, share=%p\n", ctrl->segid, ctrl, local, share);
	sem_post(&ctrl->sem1);	/* end xpmem_make */
	sem_wait(&ctrl->sem2);	/* wait xpmem_get,attach */

	for (i = 0; i < TEST_LOOP; i++) {
		ctrl->pattern = random();
		for (p = 0; p < MAP_SIZE / sizeof(long); p++)
			local[p] = ctrl->pattern;	/* fill test pattern */
		clock_gettime(CLOCK_MONOTONIC, &ctrl->t);	/* start share memory */
		if (IS_SHMEM_TEST(i))
			memcpy(share, local, MAP_SIZE);	/* copy data, if shmem test */
		sem_post(&ctrl->sem1);	/* kick receiver */
		sem_wait(&ctrl->sem2);	/* wait end share memory */
		usleep(100);
	}
	waitpid(pid, &status, 0);
	xpmem_remove(ctrl->segid);
	sem_destroy(&ctrl->sem1);
	sem_destroy(&ctrl->sem2);
	munmap(local, MAP_SIZE);
	munmap(share, MAP_SIZE);
	munmap(ctrl, sizeof(struct shmctrl));
	shm_unlink("/xpmem_share");
	shm_unlink("/xpmem_ctrl");
	return 0;
}


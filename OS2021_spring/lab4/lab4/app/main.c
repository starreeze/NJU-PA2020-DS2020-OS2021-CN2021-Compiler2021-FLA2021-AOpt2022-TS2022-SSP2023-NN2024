#include "lib.h"
#include "types.h"
#define N 5 // number of philosopher
uint32_t next;

uint32_t rand()
{
	return next = next * 1103515245 + 12345;
}

void philosopher(int i, sem_t *forks)
{
	--i; // pid start at 1
	while (1)
	{
		if (i % 2 == 0)
		{
			sem_wait(&forks[i]);
			sleep(rand() % 128);
			sem_wait(&forks[(i + 1) % N]);
		}
		else
		{
			sem_wait(&forks[(i + 1) % N]);
			sleep(rand() % 128);
			sem_wait(&forks[i]);
		}
		printf("Philosopher %d: eat\n", i);
		sleep(rand() % 128 + 64); // eat
		printf("Philosopher %d: think\n", i);
		sem_post(&forks[i]);
		sleep(rand() % 128);
		sem_post(&forks[(i + 1) % N]);
		sleep(rand() % 128 + 64); // think
	}
}

void producer(int id, sem_t *mutex, sem_t *full, sem_t *empty)
{
	--id; // pid start at 1
	while (1)
	{
		sem_wait(empty);
		sleep(rand() % 128);
		sem_wait(mutex);
		sleep(rand() % 128);
		printf("Producer %d: produce\n", id);
		sleep(rand() % 128);
		sem_post(mutex);
		sleep(rand() % 128);
		sem_post(full);
		sleep(rand() % 128);
	}
}

void consumer(sem_t *mutex, sem_t *full, sem_t *empty)
{
	while (1)
	{
		sem_wait(full);
		sleep(rand() % 128);
		sem_wait(mutex);
		sleep(rand() % 128);
		printf("Consumer: consume\n");
		sleep(rand() % 128);
		sem_post(mutex);
		sleep(rand() % 128);
		sem_post(empty);
		sleep(rand() % 128);
	}
}

void writer(sem_t *writemutex)
{
	int id = get_pid() - 4;
	while (1)
	{
		sem_wait(writemutex);
		printf("Writer %d: write\n", id);
		sleep(rand() % 128);
		sem_post(writemutex);
		sleep(rand() % 128);
	}
}

void reader(sem_t *writemutex, sem_t *countmutex)
{
	int id = get_pid() - 1;
	int rcount;
	while (1)
	{
		sem_wait(countmutex);
		sleep(rand() % 128);
		rcount = shm_read();
		if (rcount == 0)
			sem_wait(writemutex);
		sleep(rand() % 128);
		rcount = shm_read();
		++rcount;
		shm_write(rcount);
		sleep(rand() % 128);
		sem_post(countmutex);
		printf("Reader %d: read, total %d reader\n", id, rcount);
		sleep(rand() % 128);
		sem_wait(countmutex);
		rcount = shm_read();
		--rcount;
		shm_write(rcount);
		sleep(rand() % 128);
		rcount = shm_read();
		if (rcount == 0)
			sem_post(writemutex);
		sleep(rand() % 128);
		sem_post(countmutex);
		sleep(rand() % 128);
	}
}

void test_scanf()
{
	int dec = 0;
	int hex = 0;
	char str[6];
	char cha = 0;
	int ret = 0;
	while (1)
	{
		printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
		const char *bug = " Test %c Test %6s %d %x";
		ret = scanf(bug, &cha, str, &dec, &hex);
		printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
		if (ret == 4)
			break;
	}
}

void test_sem()
{
	int i = 4;

	sem_t sem;
	printf("Father Process: Semaphore Initializing.\n");
	int ret = sem_init(&sem, 2);
	if (ret == -1)
	{
		printf("Father Process: Semaphore Initializing Failed.\n");
		exit();
	}

	ret = fork();
	if (ret == 0)
	{
		while (i != 0)
		{
			i--;
			printf("Child Process: Semaphore Waiting.\n");
			sem_wait(&sem);
			printf("Child Process: In Critical Area.\n");
		}
		printf("Child Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}
	else if (ret != -1)
	{
		while (i != 0)
		{
			i--;
			printf("Father Process: Sleeping.\n");
			sleep(128);
			printf("Father Process: Semaphore Posting.\n");
			sem_post(&sem);
		}
		printf("Father Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}
}

void test_philosopher()
{
	sem_t forks[N];
	int id;
	for (int i = 0; i < N; ++i)
		sem_init(&forks[i], 1);
	for (int i = 0; i < N - 1; ++i)
	{
		int ret = fork();
		if (ret == -1)
		{
			printf("fork error: %d\n", i);
			exit();
		}
		if (ret == 0) // child
		{
			id = get_pid();
			philosopher(id, forks);
		}
	}
	id = get_pid();
	philosopher(id, forks);
}

void test_producer_consumer()
{
	sem_t mutex, full, empty;
	sem_init(&mutex, 1);
	sem_init(&full, 0);
	sem_init(&empty, N);
	int id;
	for (int i = 0; i < N - 1; ++i)
	{
		int ret = fork();
		if (ret == -1)
		{
			printf("fork error: %d\n", i);
			exit();
		}
		if (ret == 0) // child
		{
			id = get_pid();
			producer(id, &mutex, &full, &empty);
		}
	}
	id = get_pid();
	consumer(&mutex, &full, &empty);
}

void test_reader_writer()
{
	sem_t writemutex, countmutex;
	sem_init(&writemutex, 1);
	sem_init(&countmutex, 1);
	shm_write(0);
	int ret = 1;
	for (int i = 0; i < 5; ++i)
		if (ret > 0)
		{
			ret = fork();
			// printf("fork return %d in loop %d\n", ret, i);
		}
	int id = get_pid();
	if (id < 4)
	{
		// printf("reader %d\n", id - 1);
		reader(&writemutex, &countmutex);
	}
	else if (id < 7)
	{
		// printf("writer %d\n", id - 4);
		writer(&writemutex);
	}
}

int uEntry(void)
{
	// For lab4.1
	test_scanf();

	// For lab4.2
	test_sem();

	// For lab4.3
	//! note that this function is non-stopping
	test_philosopher();

	// Optional
	//! note that these functions are non-stopping
	test_producer_consumer();
	test_reader_writer();

	return 0;
}

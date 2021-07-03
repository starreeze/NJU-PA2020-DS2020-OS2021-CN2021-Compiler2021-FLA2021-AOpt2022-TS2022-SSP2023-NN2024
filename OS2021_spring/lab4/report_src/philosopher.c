#define N 5 // number of philosopher
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
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
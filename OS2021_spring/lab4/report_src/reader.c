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

void test_reader_writer()
{
	sem_t writemutex, countmutex;
	sem_init(&writemutex, 1);
	sem_init(&countmutex, 1);
	shm_write(0);
	int ret = 1;
	for (int i = 0; i < 5; ++i)
		if (ret > 0)
			ret = fork();
	int id = get_pid();
	if (id < 4)
		reader(&writemutex, &countmutex);
	else if (id < 7)
		writer(&writemutex);
}
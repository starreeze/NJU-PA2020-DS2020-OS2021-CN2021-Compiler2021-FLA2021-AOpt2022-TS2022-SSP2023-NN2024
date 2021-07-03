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
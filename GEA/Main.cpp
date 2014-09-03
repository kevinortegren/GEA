#include <iostream>
#include <random>
#include <thread>
#include "Timer.h"
#include "Memory/StackAllocator.h"

const size_t STACK_TEST_WORKER_COUNT = 8;
const size_t STACK_TEST_OBJECTS_PER_WORKER = 512;
const size_t STACK_TEST_FRAME_COUNT = 1024;

void PoolTest();
void StackTestCustom();
void StackTestTaskCustom();
void StackTestDefault();
void StackTestTaskDefault();

int main()
{
	std::cout << "Hej" << std::endl;
	std::cin.get();
	return 0;
}

/*
	Test the performance of a memory pool. This is the entry point for a thread.

	Simulate a particle system with only lifetime, that creates and destroys objects over time.
	The time for allocation and deallocation every frame will be measured.
*/
void PoolTest()
{
	Timer timer;
	bool running = true;
	while (running)
	{
		// Start timing
		timer.Start();

		// TODO: Allocate particle objects
		// TODO: Update simulation of particles (increase lived time)
		// TODO: Deallocate dead particle objects.
		// TODO: Check if all are dead and terminate.

		// Measure time.
		double elapsed = timer.Stop();

		// TODO: Store profiling data.
	}
}



/*
	Stack Test with custom memory manager.

	This will spawn a number of worker threads that will simultaneously use the memory manager.
*/
void StackTestCustom()
{
	Timer timer;
	bool running = true;
	StackMemoryManager stack(STACK_TEST_WORKER_COUNT * (STACK_TEST_OBJECTS_PER_WORKER / 2) * (STACK_TEST_OBJECTS_PER_WORKER + 1));
	std::vector<std::thread> workers;
	workers.reserve(STACK_TEST_WORKER_COUNT);

	while (running)
	{
		// Start timing.
		timer.Start();

		// Start a number of worker threads that share the stack.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			// Start worker thread.
			workers.push_back(std::thread(StackTestTaskDefault, stack));
		}

		// Join all worker threads.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			workers[i].join();
		}

		// Clear the stack.
		stack.Clear();

		// Measure time.
		double elapsed = timer.Stop();

		// TODO: Store profiling data.
	}
}

void StackTestTaskCustom(StackMemoryManager& stack)
{
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		// TODO: Allocate with memory manager.
	}
}

void StackTestDefault()
{
	Timer timer;
	bool running = true;
	std::vector<std::thread> workers;
	workers.reserve(STACK_TEST_WORKER_COUNT);

	for (size_t k = 0; k < STACK_TEST_FRAME_COUNT; ++k)
	{
		// Start timing.
		timer.Start();

		// Start a number of worker threads that allocate memory with default new.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			// Start worker thread.
			workers.push_back(std::thread(StackTestTaskDefault));
		}

		// Join all worker threads.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			workers[i].join();
		}

		// Measure time.
		double elapsed = timer.Stop();

		// TODO: Store profiling data.
	}
}

void StackTestTaskDefault()
{
	std::vector<void*> stack(STACK_TEST_OBJECTS_PER_WORKER);
	
	// Allocate the stack with default new..
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		stack[i] = new char[i + 1];
	}

	// Delete the stack.
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		delete [] stack[i];
	}
}



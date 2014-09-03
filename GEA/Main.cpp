#include <iostream>
#include <random>
#include <thread>
#include "Timer.h"
#include "Memory/StackAllocator.h"
#include "Memory/PoolAllocator.h"

const size_t STACK_TEST_WORKER_COUNT = 1;
const size_t STACK_TEST_OBJECTS_PER_WORKER = 512;
const size_t STACK_TEST_FRAME_COUNT = 1024;

const size_t POOL_TEST_SPAWN_FRAME_LIMIT = 2048;
const size_t POOL_TEST_PARTICLE_COUNT = 4096;
const size_t POOL_TEST_PARTICLE_MAX_LIFETIME = 2;

const size_t SIMPLE_POOL_TEST_FRAME_COUNT = 1024;
const size_t SIMPLE_POOL_PARTICLE_COUNT = 1024;

struct Particle
{
	Particle(int framesLeftToLive) 
	{
		this->framesLeftToLive = framesLeftToLive;
	}

	int framesLeftToLive;
};

template <typename T>
void SimplePoolTestUnthreaded(T& allocator);

template <typename T>
void PoolTestUnthreaded(T& allocator);

void StackTestCustom();
void StackTestTaskCustom(StackMemoryManager& stack);
void StackTestDefault();
void StackTestTaskDefault();

void StackTestCustomUnthreaded();
void StackTestTaskCustomSameSize(StackMemoryManager& stack);

int main()
{
	/*
	StackTestCustomUnthreaded();
	StackTestCustom();
	StackTestDefault();
	*/

	DefaultMemoryManager defaultMM(sizeof(Particle));
	PoolMemoryManager poolMM(sizeof(Particle), POOL_TEST_PARTICLE_COUNT);

	srand(13);
	//SimplePoolTestUnthreaded(poolMM);
	PoolTestUnthreaded(poolMM);
	srand(13);
	//SimplePoolTestUnthreaded(defaultMM);
	PoolTestUnthreaded(defaultMM);

	std::cout << "Hej" << std::endl;
	std::cin.get();
	return 0;
}



template <typename T>
void SimplePoolTestUnthreaded(T& allocator)
{
	Timer timer;
	std::vector<void*> ptrs;
	int frameCount = 0;
	double totalTime = 0.0;

	ptrs.reserve(SIMPLE_POOL_PARTICLE_COUNT);

	for (size_t i = 0; i < SIMPLE_POOL_TEST_FRAME_COUNT; ++i) 
	{
		// Start timing
		timer.Start();

		for (size_t k = 0; k < SIMPLE_POOL_PARTICLE_COUNT; ++k)
		{
			ptrs.push_back(allocator.Alloc());
		}

		for (size_t k = 0; k < SIMPLE_POOL_PARTICLE_COUNT; ++k)
		{
			allocator.Free(ptrs[k]);
		}

		ptrs.clear();

		// Measure time.
		double elapsed = timer.Stop();

		// Store profiling data.
		totalTime += elapsed;
		frameCount++;
	}

	std::cout << frameCount << " : " << totalTime / frameCount << std::endl;
}

/*
	Test the performance of a memory pool. This is the entry point for a thread.

	Simulate a particle system with only lifetime, that creates and destroys objects over time.
	The time for allocation and deallocation every frame will be measured.
*/
template <typename T>
void PoolTestUnthreaded(T& allocator)
{
	Timer timer;
	bool running = true;
	
	std::vector<Particle*> particles;
	int frameCount = 0;
	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;
	particles.reserve(POOL_TEST_PARTICLE_COUNT);

	while (running)
	{
		// Start timing
		timer.Start();

		// Allocate particle objects
		while (frameCount < POOL_TEST_SPAWN_FRAME_LIMIT && particles.size() < POOL_TEST_PARTICLE_COUNT)
		{
			particles.push_back(new(allocator.Alloc()) Particle(rand() % POOL_TEST_PARTICLE_MAX_LIFETIME + 2));
		}

		// Update simulation of particles (increase lived time)
		for (auto particle : particles)
		{
			particle->framesLeftToLive--;
		}

		// Deallocate dead particle objects.
		for (auto it = particles.begin(); it != particles.end();)
		{
			if ((*it)->framesLeftToLive <= 0)
			{
				allocator.Free(*it);
				it = particles.erase(it);
			}
			else
			{
				it++;
			}
		}

		// Check if all are dead and terminate.
		running = !particles.empty();

		// Measure time.
		double elapsed = timer.Stop();

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;

		// Store profiling data.
		totalTime += elapsed;
		frameCount++;
	}

	std::cout << frameCount << " : " << totalTime / frameCount << " : Min = " << minTime << " : Max = " << maxTime << std::endl;
}



/*
	Stack Test with custom memory manager.

	This will spawn a number of worker threads that will simultaneously use the memory manager.
*/
void StackTestCustom()
{
	Timer timer;
	StackMemoryManager stack(STACK_TEST_WORKER_COUNT * (STACK_TEST_OBJECTS_PER_WORKER / 2) * (STACK_TEST_OBJECTS_PER_WORKER + 1));
	std::vector<std::thread> workers;
	workers.reserve(STACK_TEST_WORKER_COUNT);

	double totalTime = 0.0;
	int frameCount = 0;

	for (size_t k = 0; k < STACK_TEST_FRAME_COUNT; ++k)
	{
		// Start timing.
		timer.Start();

		// Start a number of worker threads that share the stack.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			// Start worker thread.
			workers.push_back(std::thread(StackTestTaskCustom, std::ref(stack)));
		}

		// Join all worker threads.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			workers[i].join();
		}

		workers.clear();

		// Clear the stack.
		//std::cout << stack.allocator.GetAllocatedSize() << std::endl;
		stack.Clear();

		// Measure time.
		double elapsed = timer.Stop();

		// Store profiling data.
		totalTime += elapsed;
		frameCount++;
	}

	std::cout << totalTime / frameCount << std::endl;
}

void StackTestCustomUnthreaded()
{
	Timer timer;
	StackMemoryManager stack(STACK_TEST_WORKER_COUNT * (STACK_TEST_OBJECTS_PER_WORKER / 2) * (STACK_TEST_OBJECTS_PER_WORKER + 1));

	double totalTime = 0.0;
	int frameCount = 0;

	for (size_t k = 0; k < STACK_TEST_FRAME_COUNT; ++k)
	{
		// Start timing.
		timer.Start();

		// Start a number of worker threads that share the stack.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			StackTestTaskCustomSameSize(stack);
		}

		// Clear the stack.
		stack.Clear();

		// Measure time.
		double elapsed = timer.Stop();

		// Store profiling data.
		totalTime += elapsed;
		frameCount++;
	}

	std::cout << totalTime / frameCount << std::endl;
}

void StackTestTaskCustom(StackMemoryManager& stack)
{
	// Allocate the stack with custom memory manager.
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		char* ptr = (char*)stack.Alloc(i + 1);
	}
}

void StackTestTaskCustomSameSize(StackMemoryManager& stack)
{
	// Allocate the stack with custom memory manager.
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		char* ptr = (char*)stack.Alloc(4);
	}
}


void StackTestDefault()
{
	Timer timer;
	std::vector<std::thread> workers;
	workers.reserve(STACK_TEST_WORKER_COUNT);

	double totalTime = 0.0;
	int frameCount = 0;

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

		workers.clear();

		// Measure time.
		double elapsed = timer.Stop();

		// Store profiling data.
		totalTime += elapsed;
		frameCount++;
	}

	std::cout << totalTime / frameCount << std::endl;
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



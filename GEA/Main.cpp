#include <iostream>
#include <random>
#include <thread>
#include <fstream>
#include <cassert>
#include <sstream>
#include "Timer.h"
#include "Memory/StackAllocator.h"
#include "Memory/PoolAllocator.h"
#include "CMDColor.h"

const size_t STACK_TEST_WORKER_COUNT = 4;
const size_t STACK_TEST_OBJECTS_PER_WORKER = 2048;
const size_t STACK_TEST_FRAME_COUNT = 1000;
const size_t STACK_MAX_ALLOC_SIZE = 8192 * 4;

const size_t POOL_TEST_SPAWN_FRAME_LIMIT = 2048;
const size_t POOL_TEST_PARTICLE_COUNT = 4096;
const size_t POOL_TEST_PARTICLE_MAX_LIFETIME = 8;

const size_t POOL_TEST_THREADED_SPAWN_FRAME_LIMIT = 2048;
const size_t POOL_TEST_THREADED_PARTICLE_COUNT = 4096;
const size_t POOL_TEST_THREADED_PARTICLE_MAX_LIFETIME = 8;
const size_t POOL_TEST_THREADED_WORKER_COUNT = 4;

struct Particle
{
	Particle(int framesLeftToLive) 
	{
		this->framesLeftToLive = framesLeftToLive;
	}

	int framesLeftToLive;
	char data[8192];
};

int RND[POOL_TEST_PARTICLE_COUNT];
int RNDThreaded[POOL_TEST_THREADED_PARTICLE_COUNT];
int RNDStack[STACK_TEST_OBJECTS_PER_WORKER];

template <typename T>
void PoolTestUnthreaded(T& allocator, bool custom);

template <typename T>
void PoolTestThreaded(T& allocator, bool custom);

template <typename T>
void PoolTestTask(T& allocator, int tid, std::mutex& coutmtx, bool custom);

void MultiplePoolTestThreaded();
void MultiplePoolTestTask(int tid, std::mutex& coutmtx);

void PoolTestWriteCaptions(std::fstream& file);
void PoolTestWriteFrameData(std::fstream& file, int frameNumber, double elapsed, int creations, int deletions, int allocationSize);

double StackTestCustom();
void StackTestTaskCustom(StackMemoryManager& stack);
double StackTestDefault();
void StackTestTaskDefault();

double StackTestCustomUnthreaded();
double StackTestDefaultUnthreaded();
void StackTestTaskCustomSameSize(StackMemoryManager& stack);

int main()
{
	ColorCMD::ConsoleColorInit();
	srand(13);
	for (int i = 0; i < POOL_TEST_PARTICLE_COUNT; ++i)
	{
		RND[i] = rand() % POOL_TEST_PARTICLE_MAX_LIFETIME + 2;
	}

	for (int i = 0; i < POOL_TEST_THREADED_PARTICLE_COUNT; ++i)
	{
		RNDThreaded[i] = rand() % POOL_TEST_THREADED_PARTICLE_MAX_LIFETIME + 2;
	}

	int rndStackSum = 0;
	for(int i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		RNDStack[i] = rand() % STACK_MAX_ALLOC_SIZE + 1;
		rndStackSum += RNDStack[i];
	}

	//Print stack test parameters
	ColorCMD::SetTextColor(ColorCMD::ConsoleColor::AQUA);
	std::cout << "STACK_TEST_WORKER_COUNT: " << STACK_TEST_WORKER_COUNT << std::endl;
	std::cout << "STACK_TEST_OBJECTS_PER_WORKER: " << STACK_TEST_OBJECTS_PER_WORKER << std::endl;
	std::cout << "STACK_TEST_FRAME_COUNT: " << STACK_TEST_FRAME_COUNT << std::endl;
	std::cout << "STACK_MAX_ALLOC_SIZE: " << STACK_MAX_ALLOC_SIZE << std::endl;
	std::cout << "STACK_ALLOCATION_PER_FRAME: " << rndStackSum * STACK_TEST_WORKER_COUNT << std::endl;

	std::cout << std::endl;
	ColorCMD::SetTextColor(ColorCMD::ConsoleColor::WHITE);

	std::cout << "-- Stack Test Unthreaded (Custom) --" << std::endl; double stackTestCustomTimeAvg = StackTestCustomUnthreaded();	std::cout << std::endl;	
	std::cout << "-- Stack Test Unthreaded (Default) --" << std::endl; double stackTestDefaultTimeAvg = StackTestDefaultUnthreaded(); std::cout << std::endl;

	std::cout << "Average Frame Time Difference: " << abs(stackTestCustomTimeAvg - stackTestDefaultTimeAvg) << std::endl << std::endl;

	std::cout << "-- Stack Test Threaded (Custom) --" << std::endl;	 double stackTestCustomThreadedAvg = StackTestCustom();		std::cout << std::endl;
	std::cout << "-- Stack Test Threaded (Default) --" << std::endl;  double stackTestDefaultThreadedAvg = StackTestDefault();	std::cout << std::endl;

	std::cout << "Average Frame Time Difference: " << abs(stackTestCustomThreadedAvg - stackTestDefaultThreadedAvg) << std::endl << std::endl;

	DefaultMemoryManager defaultMM(sizeof(Particle));
	PoolAllocator poolMM(sizeof(Particle), POOL_TEST_PARTICLE_COUNT);
	ThreadedPoolAllocator threadedPoolMM(sizeof(Particle), POOL_TEST_THREADED_PARTICLE_COUNT * POOL_TEST_THREADED_WORKER_COUNT);

	//Print pool test parameters
	ColorCMD::SetTextColor(ColorCMD::ConsoleColor::AQUA);
	std::cout << "POOL_TEST_SPAWN_FRAME_LIMIT: " << POOL_TEST_SPAWN_FRAME_LIMIT << std::endl;
	std::cout << "POOL_TEST_PARTICLE_COUNT: " << POOL_TEST_PARTICLE_COUNT << std::endl;
	std::cout << "POOL_TEST_PARTICLE_MAX_LIFETIME: " << POOL_TEST_PARTICLE_MAX_LIFETIME << std::endl;
	std::cout << std::endl;
	ColorCMD::SetTextColor(ColorCMD::ConsoleColor::WHITE);

	std::cout << "-- Pool Test Unthreaded (Custom) --" << std::endl;				PoolTestUnthreaded(poolMM, true);				std::cout << std::endl;
	std::cout << "-- Pool Test Unthreaded (Default) --" << std::endl;				PoolTestUnthreaded(defaultMM, false);				std::cout << std::endl;
	std::cout << std::endl;

	ColorCMD::SetTextColor(ColorCMD::ConsoleColor::AQUA);
	std::cout << "POOL_TEST_THREADED_SPAWN_FRAME_LIMIT: " << POOL_TEST_THREADED_SPAWN_FRAME_LIMIT << std::endl;
	std::cout << "POOL_TEST_THREADED_PARTICLE_COUNT: " << POOL_TEST_THREADED_PARTICLE_COUNT << std::endl;
	std::cout << "POOL_TEST_THREADED_PARTICLE_MAX_LIFETIME: " << POOL_TEST_THREADED_PARTICLE_MAX_LIFETIME << std::endl;
	std::cout << "POOL_TEST_THREADED_WORKER_COUNT: " << POOL_TEST_THREADED_WORKER_COUNT << std::endl;
	std::cout << std::endl;
	ColorCMD::SetTextColor(ColorCMD::ConsoleColor::WHITE);

	std::cout << "-- Pool Test Threaded (Custom) --" << std::endl;					PoolTestThreaded(threadedPoolMM, true);			std::cout << std::endl;
	std::cout << "-- Pool Test Threaded (Default) --" << std::endl;					PoolTestThreaded(defaultMM, false);				std::cout << std::endl;

	std::cout << "-- Multiple Pool Test Threaded (Custom) --" << std::endl;			MultiplePoolTestThreaded();				std::cout << std::endl;

	std::cout << "Hej" << std::endl;
	std::cin.get();
	return 0;
}

/*
	Test the performance of a memory pool. This is the entry point for a thread.

	Simulate a particle system with only lifetime, that creates and destroys objects over time.
	The time for allocation and deallocation every frame will be measured.
*/
template <typename T>
void PoolTestUnthreaded(T& allocator, bool custom)
{
	std::stringstream ss;
	ss << "pool_unthreaded_" << (custom ? "custom" : "default") << ".csv";
	
	std::fstream file;
	file.open(ss.str(), std::ios_base::trunc | std::ios_base::out);
	PoolTestWriteCaptions(file);

	Timer frameTimer;
	bool running = true;
	
	size_t freeList[POOL_TEST_PARTICLE_COUNT];
	Particle* particles[POOL_TEST_PARTICLE_COUNT];

	for(unsigned i = 0; i < POOL_TEST_PARTICLE_COUNT; ++i)
		freeList[i] = i;

	int freeListIndex = POOL_TEST_PARTICLE_COUNT - 1;
	int frameCount = 0;
	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;

	while (running)
	{
		int creations = 0;
		int deletions = 0;

		// Start timing
		frameTimer.Start();

		// Allocate particle objects
		while (frameCount < POOL_TEST_SPAWN_FRAME_LIMIT && freeListIndex != -1)
		{
			creations++;

			int lifetime = RND[freeList[freeListIndex]];
			Particle* p = new(allocator.Alloc()) Particle(lifetime);
			
			particles[freeList[freeListIndex--]] = p;
		}

		// Update simulation of particles (increase lived time)
		// Deallocate dead particle objects.
		for (size_t i = 0; i < POOL_TEST_PARTICLE_COUNT; ++i)
		{
			Particle*& particle = particles[i];

			if(particle != nullptr)
			{
				particle->framesLeftToLive--;

				if (particle->framesLeftToLive <= 0)
				{
					deletions++;

					allocator.Free(particle);

					particle = nullptr;
					freeList[++freeListIndex] = i;
				}
			}
		}

		// Check if all are dead and terminate.
		running = (freeListIndex != POOL_TEST_PARTICLE_COUNT - 1) || (frameCount < POOL_TEST_SPAWN_FRAME_LIMIT);

		// Measure time.
		double elapsed = frameTimer.Stop();

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;

		// Store profiling data.
		PoolTestWriteFrameData(file, frameCount, elapsed, creations, deletions, creations * sizeof(Particle));

		totalTime += elapsed;
		frameCount++;
	}

	std::cout << "Frames Simulated: " << frameCount << std::endl;
	std::cout << "Total Experiment Time: " << totalTime << std::endl;
	std::cout << "Average Frame Time: " << totalTime / frameCount << std::endl;
	std::cout << "Min Frame Time: " << minTime << std::endl;
	std::cout << "Max Frame Time: " << maxTime << std::endl;
}

template <typename T>
void PoolTestThreaded(T& allocator, bool custom)
{
	std::mutex coutmtx;
	std::vector<std::thread> workers;
	workers.reserve(POOL_TEST_THREADED_WORKER_COUNT);

	for (int k = 0; k < POOL_TEST_THREADED_WORKER_COUNT; ++k)
	{
		workers.push_back(std::thread(PoolTestTask<T>, std::ref(allocator), k, std::ref(coutmtx), custom));
	}

	for (int k = 0; k < POOL_TEST_THREADED_WORKER_COUNT; ++k)
	{
		workers[k].join();
	}
}

/*
	Runs a particle system of its own.
*/
template <typename T>
void PoolTestTask(T& allocator, int tid, std::mutex& coutmtx, bool custom)
{
	std::stringstream ss;
	ss << "pool_threaded_" << (custom ? "custom" : "default") << "_" << tid << ".csv";

	std::fstream file;
	file.open(ss.str(), std::ios_base::trunc | std::ios_base::out);
	PoolTestWriteCaptions(file);

	// Setup particle list and free-index list.
	size_t freeList[POOL_TEST_THREADED_PARTICLE_COUNT];
	Particle* particles[POOL_TEST_THREADED_PARTICLE_COUNT];

	for(unsigned i = 0; i < POOL_TEST_THREADED_PARTICLE_COUNT; ++i)
		freeList[i] = i;

	int freeListIndex = POOL_TEST_THREADED_PARTICLE_COUNT - 1;

	Timer timer;
	int frameCount = 0;
	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;

	// Start the simulation
	bool running = true;
	while (running)
	{
		int creations = 0;
		int deletions = 0;

		timer.Start();

		// Allocate particle objects
		while (frameCount < POOL_TEST_THREADED_SPAWN_FRAME_LIMIT && freeListIndex != -1)
		{
			creations++;

			int lifetime = RNDThreaded[freeList[freeListIndex]];
			Particle* p = new(allocator.Alloc()) Particle(lifetime);
			
			particles[freeList[freeListIndex--]] = p;
		}

		// Update simulation of particles (increase lived time)
		// Deallocate dead particle objects.
		for (size_t i = 0; i < POOL_TEST_THREADED_PARTICLE_COUNT; ++i)
		{
			Particle*& particle = particles[i];

			if(particle != nullptr)
			{
				particle->framesLeftToLive--;

				if (particle->framesLeftToLive <= 0)
				{
					deletions++;

					allocator.Free(particle);

					particle = nullptr;
					freeList[++freeListIndex] = i;
				}
			}
		}

		// Check if all are dead and terminate.
		running = (freeListIndex != POOL_TEST_THREADED_PARTICLE_COUNT - 1)  || (frameCount < POOL_TEST_THREADED_SPAWN_FRAME_LIMIT);

		double elapsed = timer.Stop();
		frameCount++;
		totalTime += elapsed;

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;

		PoolTestWriteFrameData(file, frameCount, elapsed, creations, deletions, creations * sizeof(Particle));
	}

	std::lock_guard<std::mutex> lock(coutmtx);
	std::cout << "Thread " << tid << std::endl;
	std::cout << "\tFrames Simulated: " << frameCount << std::endl;
	std::cout << "\tTotal Experiment Time: " << totalTime << std::endl;
	std::cout << "\tAverage Frame Time: " << totalTime / frameCount << std::endl;
	std::cout << "\tMin Frame Time: " << minTime << std::endl;
	std::cout << "\tMax Frame Time: " << maxTime << std::endl;
}

void MultiplePoolTestThreaded()
{
	std::mutex coutmtx;
	std::vector<std::thread> workers;
	workers.reserve(POOL_TEST_THREADED_WORKER_COUNT);

	for (int k = 0; k < POOL_TEST_THREADED_WORKER_COUNT; ++k)
	{
		workers.push_back(std::thread(MultiplePoolTestTask, k, std::ref(coutmtx)));
	}

	for (int k = 0; k < POOL_TEST_THREADED_WORKER_COUNT; ++k)
	{
		workers[k].join();
	}
}

void MultiplePoolTestTask( int tid, std::mutex& coutmtx)
{
	std::stringstream ss;
	ss << "multiple_pool_threaded_custom_" << tid << ".csv";

	std::fstream file;
	file.open(ss.str(), std::ios_base::trunc | std::ios_base::out);
	
	PoolTestWriteCaptions(file);

	PoolAllocator allocator(sizeof(Particle), POOL_TEST_THREADED_PARTICLE_COUNT);

	// Setup particle list and free-index list.
	size_t freeList[POOL_TEST_THREADED_PARTICLE_COUNT];
	Particle* particles[POOL_TEST_THREADED_PARTICLE_COUNT];

	for(unsigned i = 0; i < POOL_TEST_THREADED_PARTICLE_COUNT; ++i)
		freeList[i] = i;

	int freeListIndex = POOL_TEST_THREADED_PARTICLE_COUNT - 1;

	Timer timer;
	int frameCount = 0;
	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;

	// Start the simulation
	bool running = true;
	while (running)
	{
		int creations = 0;
		int deletions = 0;

		timer.Start();

		// Allocate particle objects
		while (frameCount < POOL_TEST_THREADED_SPAWN_FRAME_LIMIT && freeListIndex != -1)
		{
			creations++;

			int lifetime = RNDThreaded[freeList[freeListIndex]];
			Particle* p = new(allocator.Alloc()) Particle(lifetime);
			
			particles[freeList[freeListIndex--]] = p;
		}

		// Update simulation of particles (increase lived time)
		// Deallocate dead particle objects.
		for (size_t i = 0; i < POOL_TEST_THREADED_PARTICLE_COUNT; ++i)
		{
			Particle*& particle = particles[i];

			if(particle != nullptr)
			{
				particle->framesLeftToLive--;

				if (particle->framesLeftToLive <= 0)
				{
					deletions++;

					allocator.Free(particle);

					particle = nullptr;
					freeList[++freeListIndex] = i;
				}
			}
		}

		// Check if all are dead and terminate.
		running = (freeListIndex != POOL_TEST_THREADED_PARTICLE_COUNT - 1)  || (frameCount < POOL_TEST_THREADED_SPAWN_FRAME_LIMIT);

		double elapsed = timer.Stop();
		frameCount++;
		totalTime += elapsed;

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;

		PoolTestWriteFrameData(file, frameCount, elapsed, creations, deletions, creations * sizeof(Particle));
	}

	std::lock_guard<std::mutex> lock(coutmtx);
	std::cout << "Thread " << tid << std::endl;
	std::cout << "\tFrames Simulated: " << frameCount << std::endl;
	std::cout << "\tTotal Experiment Time: " << totalTime << std::endl;
	std::cout << "\tAverage Frame Time: " << totalTime / frameCount << std::endl;
	std::cout << "\tMin Frame Time: " << minTime << std::endl;
	std::cout << "\tMax Frame Time: " << maxTime << std::endl;
}

void PoolTestWriteCaptions(std::fstream& file)
{
	file << "Frame; Time; Creations; Deletions; Allocation Size" << std::endl;
}

void PoolTestWriteFrameData(std::fstream& file, int frameNumber, double elapsed, int creations, int deletions, int allocationSize)
{
	file << frameNumber << "; " << elapsed << "; " << creations << "; " << deletions << "; " << creations * sizeof(Particle) << std::endl;
}

/*
	Stack Test with custom memory manager.

	This will spawn a number of worker threads that will simultaneously use the memory manager.
*/
double StackTestCustom()
{
	Timer timer;
	StackMemoryManager stack(STACK_TEST_WORKER_COUNT * STACK_TEST_OBJECTS_PER_WORKER * STACK_MAX_ALLOC_SIZE);
	std::vector<std::thread> workers;
	workers.reserve(STACK_TEST_WORKER_COUNT);

	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;
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

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;
	}

	double avgFrameTime = totalTime / frameCount;
	std::cout << "Average Frame Time: " << avgFrameTime << std::endl;
	std::cout << "Min Frame Time: " << minTime << std::endl;
	std::cout << "Max Frame Time: " << maxTime << std::endl;
	return avgFrameTime;
}

double StackTestCustomUnthreaded()
{
	Timer timer;
	StackMemoryManager stack(STACK_TEST_WORKER_COUNT * STACK_TEST_OBJECTS_PER_WORKER * STACK_MAX_ALLOC_SIZE);

	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;
	int frameCount = 0;

	for (size_t k = 0; k < STACK_TEST_FRAME_COUNT; ++k)
	{
		// Start timing.
		timer.Start();

		// Start a number of worker threads that share the stack.
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			StackTestTaskCustom(stack);
		}

		// Clear the stack.
		stack.Clear();

		// Measure time.
		double elapsed = timer.Stop();

		// Store profiling data.
		totalTime += elapsed;
		frameCount++;

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;
	}

	double avgFrameTime = totalTime / frameCount;
	std::cout << "Average Frame Time: " << avgFrameTime << std::endl;
	std::cout << "Min Frame Time: " << minTime << std::endl;
	std::cout << "Max Frame Time: " << maxTime << std::endl;
	return avgFrameTime;
}


double StackTestDefault()
{
	Timer timer;
	std::vector<std::thread> workers;
	workers.reserve(STACK_TEST_WORKER_COUNT);

	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;
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

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;
	}

	double avgFrameTime = totalTime / frameCount;
	std::cout << "Average Frame Time: " << avgFrameTime << std::endl;
	std::cout << "Min Frame Time: " << minTime << std::endl;
	std::cout << "Max Frame Time: " << maxTime << std::endl;
	return avgFrameTime;
}


void StackTestTaskCustom(StackMemoryManager& stack)
{
	// Allocate the stack with custom memory manager.
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		//char* ptr = (char*)stack.Alloc(i + 1);
		char* ptr = (char*)stack.Alloc(RNDStack[i]);
	}
}


void StackTestTaskDefault()
{
	std::vector<void*> stack(STACK_TEST_OBJECTS_PER_WORKER);
	
	// Allocate the stack with default new..
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		//stack[i] = new char[i + 1];
		stack[i] = new char[RNDStack[i]];
	}

	// Delete the stack.
	for (size_t i = 0; i < STACK_TEST_OBJECTS_PER_WORKER; ++i)
	{
		delete [] stack[i];
	}
}


double StackTestDefaultUnthreaded()
{
	Timer timer;
	double totalTime = 0.0;
	double minTime = +100000000.0;
	double maxTime = -100000000.0;
	int frameCount = 0;

	for (size_t k = 0; k < STACK_TEST_FRAME_COUNT; ++k)
	{
		// Start timing.
		timer.Start();
	
		// Allocate the stack with default new..
		for (size_t i = 0; i < STACK_TEST_WORKER_COUNT; ++i)
		{
			StackTestTaskDefault();
		}

		// Measure time.
		double elapsed = timer.Stop();

		// Store profiling data.
		totalTime += elapsed;
		frameCount++;

		if (elapsed < minTime)
			minTime = elapsed;
		if (elapsed > maxTime)
			maxTime = elapsed;
	}

	double avgFrameTime = totalTime / frameCount;
	std::cout << "Average Frame Time: " << avgFrameTime << std::endl;
	std::cout << "Min Frame Time: " << minTime << std::endl;
	std::cout << "Max Frame Time: " << maxTime << std::endl;
	return avgFrameTime;

}

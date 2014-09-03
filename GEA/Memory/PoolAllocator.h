#pragma once

class FreeList
{
public:
	FreeList();
	void Initialize(void* start, unsigned elementSize, unsigned numElements);
	void Free();
	void* Obtain();
	void Lose(void* ptr);

private:
	void* m_start;
	FreeList* m_next;
};



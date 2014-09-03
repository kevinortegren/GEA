#include "PoolAllocator.h"
#include <malloc.h>

FreeList::FreeList()
	: m_start(nullptr), m_next(nullptr)
{

}

void FreeList::Initialize(void* start, unsigned elementSize, unsigned numElements)
{
	m_start = start;

	union 
	{
		void* as_void;
		char* as_char;
		FreeList* as_self;
	};

	as_void = start;
	m_next = as_self;

	FreeList* runner = m_next;
	for(unsigned i = 0; i < numElements; ++i)
	{	
		runner->m_next = as_self;
		runner = as_self;
		as_char += elementSize;
	}

	runner->m_next = nullptr;
	FreeList* completeList = (FreeList*) start;
};

void FreeList::Free()
{
    free(m_start);
}

void* FreeList::Obtain()
{
	// Reached the end of list return nullptr.
	if (m_next == nullptr) {
		return nullptr;
	}
 
	FreeList* head = m_next;
	m_next = head->m_next;
	return head;
}
 
void FreeList::Lose(void* ptr)
{
	FreeList* head = static_cast<FreeList*>(ptr);
	head->m_next = m_next;
	m_next = head;
}


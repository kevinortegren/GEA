#pragma once

class StackAllocator
{
public:
	StackAllocator(unsigned int stackSize_bytes);
	~StackAllocator();
	void* Alloc(unsigned int size_bytes);
	void Rewind( void* ptr );
	void* GetPointer(void) const;
	void* GetBegin(void) const;	
	void Clear( void );

	unsigned int GetTotalSize() const;
	unsigned int GetAllocatedSize() const;

private:
	void* m_mem;
	void* m_ptr;
	unsigned int m_stackSize_bytes;
};



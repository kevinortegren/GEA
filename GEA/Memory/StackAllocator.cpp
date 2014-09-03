#include <Memory/StackAllocator.h>
#include <iostream>
#include <assert.h>

namespace Utility
{
    Memory::StackAllocator::StackAllocator( unsigned int stackSize_bytes )
	: m_stackSize_bytes(stackSize_bytes)
    {
        m_mem = malloc( stackSize_bytes );
        m_ptr = m_mem;
    }

    Memory::StackAllocator::~StackAllocator()
    {
        if(m_mem != 0) {
            free(m_mem);
            m_mem = 0;
        }
    }

    void* Memory::StackAllocator::Alloc( unsigned int size_bytes )
    {
        assert(m_ptr < ((char*)m_mem + m_stackSize_bytes) && "Stack allocator overflow");

        void* ptr = m_ptr;
        m_ptr = (char*)m_ptr + size_bytes;

        return ptr;
    }

    void Memory::StackAllocator::Rewind( void* ptr )
    {
        m_ptr = ptr;
    }

    void* Memory::StackAllocator::GetPointer( void ) const
    {
        return m_ptr;
    }

    void Memory::StackAllocator::Clear( void )
    {
        m_ptr = m_mem;
    }

    void* Memory::StackAllocator::GetBegin( void ) const
    {
        return m_mem;
    }

    unsigned int Memory::StackAllocator::GetTotalSize() const
    {
        return m_stackSize_bytes;
    }

    unsigned int Memory::StackAllocator::GetAllocatedSize() const
    {
        return ((int)m_ptr - (int)m_mem);
    }

    Memory::StackAllocator& Memory::operator<<( Memory::StackAllocator& out, unsigned int value )
    {
        unsigned int* ptr = (unsigned int*)out.Alloc(sizeof(value));
        *ptr = value;
        return out;
    }
}


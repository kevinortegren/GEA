#pragma once

#include <Utility/include/Utility_Helper.h>

namespace Utility
{
	namespace Memory
	{
		class StackAllocator
		{
		public:
			UTILITY_API StackAllocator(unsigned int stackSize_bytes);
			UTILITY_API ~StackAllocator();
			UTILITY_API void* Alloc(unsigned int size_bytes);
			UTILITY_API void Rewind( void* ptr );
			UTILITY_API void* GetPointer(void) const;
			UTILITY_API void* GetBegin(void) const;	
			UTILITY_API void Clear( void );

			UTILITY_API unsigned int GetTotalSize() const;
			UTILITY_API unsigned int GetAllocatedSize() const;

		private:
			void* m_mem;
			void* m_ptr;
			unsigned int m_stackSize_bytes;
		};

		UTILITY_API StackAllocator& operator<<(StackAllocator& out, unsigned int value);
	}
}


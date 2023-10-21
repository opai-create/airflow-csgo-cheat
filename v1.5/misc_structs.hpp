#pragma once

#pragma region UTLVECTOR
__forceinline static int calc_new_allocation_count(int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem)
{
	if (nGrowSize)
		nAllocationCount = ((1 + ((nNewSize - 1) / nGrowSize)) * nGrowSize);
	else
	{
		if (!nAllocationCount)
			nAllocationCount = (31 + nBytesItem) / nBytesItem;

		while (nAllocationCount < nNewSize)
			nAllocationCount *= 2;
	}

	return nAllocationCount;
}

template< class T, class I = int >
class c_utl_memory
{
public:
	bool is_valid_index(I i) const
	{
		long x = i;
		return (x >= 0) && (x < alloc_count);
	}

	T& operator[](I i);
	const T& operator[](I i) const;

	T* base() {
		return memory;
	}

	int num_allocated() const
	{
		return alloc_count;
	}

	void grow(int num) {
		//assert(num > 0);

		auto old_alloc_count = alloc_count;
		// Make sure we have at least numallocated + num allocations.
		// Use the grow rules specified for this memory (in m_grow_size)
		int alloc_requested = alloc_count + num;

		int new_alloc_count = calc_new_allocation_count(alloc_count, grow_size, alloc_requested, sizeof(T));

		// if m_alloc_requested wraps index type I, recalculate
		if ((int)(I)new_alloc_count < alloc_requested)
		{
			if ((int)(I)new_alloc_count == 0 && (int)(I)(new_alloc_count - 1) >= alloc_requested)
				--new_alloc_count; // deal w/ the common case of m_alloc_count == MAX_USHORT + 1
			else
			{
				if ((int)(I)alloc_requested != alloc_requested)
				{
					// we've been asked to grow memory to a size s.t. the index type can't address the requested amount of memory
				//	assert(0);
					return;
				}
				while ((int)(I)new_alloc_count < alloc_requested)
				{
					new_alloc_count = (new_alloc_count + alloc_requested) / 2;
				}
			}
		}

		alloc_count = new_alloc_count;

		if (memory)
		{
			auto ptr = new unsigned char[alloc_count * sizeof(T)];

			std::memcpy(ptr, memory, old_alloc_count * sizeof(T));
			memory = (T*)ptr;
		}
		else
			memory = (T*)new unsigned char[alloc_count * sizeof(T)];
	}

protected:
	T* memory;
	int alloc_count;
	int grow_size;
};

template< class T, class I >
T& c_utl_memory< T, I >::operator[](I i)
{
	return memory[i];
}

template< class T, class I >
const T& c_utl_memory< T, I >::operator[](I i) const
{
	return memory[i];
}

template< class T >
void destruct(T* memory)
{
	memory->~T();
}

template< class T >
T* construct(T* memory)
{
	return ::new(memory) T;
}

template< class T >
T* copy_construct(T* memory, T const& src)
{
	return ::new(memory) T(src);
}

template< class T, class A = c_utl_memory< T > >
class c_utl_vector
{
	typedef A c_allocator;

	typedef T* iterator;
	typedef const T* const_iterator;
public:
	T& operator[](int i);
	const T& operator[](int i) const;

	T& element(int i)
	{
		return memory[i];
	}

	const T& element(int i) const
	{
		return memory[i];
	}

	T* base() {
		return memory.base();
	}

	int count() const
	{
		return size;
	}

	void remove_count()
	{
		size = 0;
	}

	void remove_all()
	{
		for (int i = size; --i >= 0; )
			destruct(&element(i));

		size = 0;
	}

	bool is_valid_index(int i) const {
		return (i >= 0) && (i < size);
	}

	void shift_elements_right(int elem, int num = 1)
	{
		int num_to_move = size - elem - num;
		if ((num_to_move > 0) && (num > 0))
			std::memmove(&element(elem + num), &element(elem), num_to_move * sizeof(T));
	}

	void shift_elements_left(int elem, int num = 1)
	{
		int numToMove = size - elem - num;
		if ((numToMove > 0) && (num > 0))
			std::memmove(&element(elem), &element(elem + num), numToMove * sizeof(T));
	}

	void grow_vector(int num = 1)
	{
		if (size + num > memory.num_allocated()) {
			memory.grow(size + num - memory.num_allocated());
		}

		size += num;
	}

	int insert_before(int elem)
	{
		grow_vector();
		shift_elements_right(elem);
		construct(&element(elem));
		return elem;
	}

	int add_to_head()
	{
		return insert_before(0);
	}

	int add_to_tail()
	{
		return insert_before(size);
	}

	int insert_before(int elem, const T& src)
	{
		grow_vector();
		shift_elements_right(elem);
		copy_construct(&element(elem), src);
		return elem;
	}

	int add_to_tail(const T& src)
	{
		return insert_before(size, src);
	}

	int find(const T& src) const
	{
		for (int i = 0; i < count(); ++i)
		{
			if (element(i) == src)
				return i;
		}
		return -1;
	}

	void remove(int elem)
	{
		destruct(&element(elem));
		shift_elements_left(elem);
		--size;
	}

	bool find_and_remove(const T& src)
	{
		int elem = find(src);
		if (elem != -1)
		{
			remove(elem);
			return true;
		}
		return false;
	}

	iterator begin()
	{
		return base();
	}

	const_iterator begin() const
	{
		return base();
	}

	iterator end()
	{
		return base() + count();
	}

	const_iterator end() const
	{
		return base() + count();
	}

protected:
	c_allocator memory;
	int size;
	T* elements;
};

template< typename T, class A >
T& c_utl_vector< T, A >::operator[](int i)
{
	return memory[i];
}

template< typename T, class A >
const T& c_utl_vector< T, A >::operator[](int i) const
{
	return memory[i];
}

struct ret_counted_t {
private:
	volatile long m_ref_count;
public:
	virtual void destructor(char bDelete) = 0;
	virtual bool on_final_release() = 0;

	void unref() {
		if (InterlockedDecrement(&m_ref_count) == 0 && on_final_release())
			destructor(1);
	}
};
#pragma endregion
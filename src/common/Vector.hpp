#ifndef __VECTOR_HPP__
#define __VECTOR_HPP__

template<typename T>
class Vector
{
private:
    uint64_t capacity;
    uint64_t size;
    T* array;

	void MakeCapacity(uint64_t newSize)
    {
        capacity = newSize;
        T* n = (T*)new char[newSize * sizeof(T)];

        if(size != 0)
        {
            for(uint64_t i = 0; i < size; i++)
            {
                new(&n[i]) T((T&&)array[i]);
                array[i].~T();
            }
            delete[] (char*)array;
        }

        array = n;
    }
	
public:
    class Iterator
    {
        uint64_t pos;
        T* data;
		
    public:
        Iterator(T* data, uint64_t pos) : data(data), pos(pos) { }

        Iterator& operator++() { pos++; }
        Iterator& operator--() { pos--; }

        T& operator*	() { return data[pos]; }
        T* operator->	() { return &data[pos]; }

        bool operator== (const Iterator& r) const { return pos == r.pos ; 	}
        bool operator!= (const Iterator& r) const { return !(*this == r);	}
		
		uint64_t Pos() { return pos; }
    };

	Vector() : capacity(0), size(0), array(nullptr) {}
	
    ~Vector()
    {
        if(capacity != 0)
            delete[] array;
    }

    Vector(const Vector& r)
    {
        capacity = 0;
        size = 0;
        MakeCapacity(r.size);

        size = r.size;
        for(uint64_t i = 0; i < size; i++)
            new(&array[i]) T(r.array[i]);
    }
	
    Vector(Vector&& r) : capacity(r.capacity), size(r.size), array(r.array)
    {
        r.capacity = 0;
		r.size = 0;
        r.array = nullptr;
    }

	T& operator[] (uint64_t index) {
		if (index >= capacity || index < 0)
			return nullptr;
        return array[index];
    }

	uint64_t Size() const { return size; }

    Iterator begin()	{ return Iterator(array, 0		); }
    Iterator end()		{ return Iterator(array, size	); }
	Iterator Begin()	{ return Iterator(array, 0		); }
    Iterator End()		{ return Iterator(array, size	); }
	
	void PushBack(const T& t)
    {
        if(capacity < size + 1) {
            MakeCapacity(size + 10);
        }

        new(&array[size]) T(t);
        size++;
    }
    
    void Erase(const Iterator& it)
    {
        array[it.Pos()].~T();
        for(uint64_t i = it.Pos() + 1; i < size; i++) {
            new(&array[i - 1]) T((T&&)array[i]);
            array[i].~T();
        }
		size--;
    }
}

#endif
#pragma once
#include <types.h>
#include <cstring>

template <class T>
class Vector
{
private:
    size_t VectorSize = 0;
    size_t VectorCapacity = 0;
    T *VectorBuffer = nullptr;

public:
    typedef T *iterator;

    __no_instrument_function Vector()
    {
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( )");
#endif
        VectorCapacity = 0;
        VectorSize = 0;
        VectorBuffer = 0;
    }

    __no_instrument_function Vector(size_t Size)
    {
        VectorCapacity = Size;
        VectorSize = Size;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( %lld )", Size);
#endif
        VectorBuffer = new T[Size];
    }

    __no_instrument_function Vector(size_t Size, const T &Initial)
    {
        VectorSize = Size;
        VectorCapacity = Size;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( %lld %llx )", Size, Initial);
#endif
        VectorBuffer = new T[Size];
        for (size_t i = 0; i < Size; i++)
            VectorBuffer[i] = Initial;
    }

    __no_instrument_function Vector(const Vector<T> &Vector)
    {
        VectorSize = Vector.VectorSize;
        VectorCapacity = Vector.VectorCapacity;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: Vector( <vector> )->Size: %lld", VectorSize);
#endif
        VectorBuffer = new T[VectorSize];
        for (size_t i = 0; i < VectorSize; i++)
            VectorBuffer[i] = Vector.VectorBuffer[i];
    }

    __no_instrument_function ~Vector()
    {
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR INIT: ~Vector( ~%lx )", VectorBuffer);
#endif
        delete[] VectorBuffer;
    }

    __no_instrument_function void remove(size_t Position)
    {
        if (Position >= VectorSize)
            return;
        memset(&*(VectorBuffer + Position), 0, sizeof(T));
        for (size_t i = 0; i < VectorSize - 1; i++)
        {
            *(VectorBuffer + Position + i) = *(VectorBuffer + Position + i + 1);
        }
        VectorSize--;
    }

    __no_instrument_function size_t capacity() const { return VectorCapacity; }

    __no_instrument_function size_t size() const { return VectorSize; }

    __no_instrument_function bool empty() const;

    __no_instrument_function iterator begin() { return VectorBuffer; }

    __no_instrument_function iterator end() { return VectorBuffer + size(); }

    __no_instrument_function T &front() { return VectorBuffer[0]; }

    __no_instrument_function T &back() { return VectorBuffer[VectorSize - 1]; }

    __no_instrument_function void push_back(const T &Value)
    {
        if (VectorSize >= VectorCapacity)
            reserve(VectorCapacity + 5);
        VectorBuffer[VectorSize++] = Value;
    }

    __no_instrument_function void pop_back() { VectorSize--; }

    __no_instrument_function void reverse()
    {
        if (VectorSize <= 1)
            return;
        for (size_t i = 0, j = VectorSize - 1; i < j; i++, j--)
        {
            T c = *(VectorBuffer + i);
            *(VectorBuffer + i) = *(VectorBuffer + j);
            *(VectorBuffer + j) = c;
        }
    }

    __no_instrument_function void reserve(size_t Capacity)
    {
        if (VectorBuffer == 0)
        {
            VectorSize = 0;
            VectorCapacity = 0;
        }
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR ALLOCATION: reverse( %lld )", Capacity);
#endif
        T *NewBuffer = new T[Capacity];
        size_t _Size = Capacity < VectorSize ? Capacity : VectorSize;
        for (size_t i = 0; i < _Size; i++)
            NewBuffer[i] = VectorBuffer[i];
        VectorCapacity = Capacity;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR ALLOCATION: reverse( <Capacity> )->Buffer:~%lld", VectorBuffer);
#endif
        delete[] VectorBuffer;
        VectorBuffer = NewBuffer;
    }

    __no_instrument_function void resize(size_t Size)
    {
        reserve(Size);
        VectorSize = Size;
    }

    __no_instrument_function T &operator[](size_t Index) { return VectorBuffer[Index]; }

    __no_instrument_function Vector<T> &operator=(const Vector<T> &Vector)
    {
        delete[] VectorBuffer;
        VectorSize = Vector.VectorSize;
        VectorCapacity = Vector.VectorCapacity;
#ifdef DEBUG_MEM_ALLOCATION
        debug("VECTOR ALLOCATION: operator=( <vector> )->Size:%lld", VectorSize);
#endif
        VectorBuffer = new T[VectorSize];
        for (size_t i = 0; i < VectorSize; i++)
            VectorBuffer[i] = Vector.VectorBuffer[i];
        return *this;
    }

    __no_instrument_function void clear()
    {
        VectorCapacity = 0;
        VectorSize = 0;
        VectorBuffer = 0;
    }

    __no_instrument_function T *data() { return VectorBuffer; }
};

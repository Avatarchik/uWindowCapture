#pragma once

#include <memory>


template <class T>
class Buffer
{
public:
    Buffer() {}

    Buffer(const Buffer& other)
    {
        value_.reset();
        size_ = 0;
        ExpandIfNeeded(other.size_);
        memcpy_s(value_.get(), size_, other.value_.get(), other.size_);
    }

    Buffer<T>& operator=(const Buffer& other)
    {
        if (&other == this) return *this;

        value_.reset();
        size_ = 0;
        ExpandIfNeeded(other.size_);
        memcpy_s(value_.get(), size_, other.value_.get(), other.size_);

        return *this;
    }

    ~Buffer() {}

    void ExpandIfNeeded(UINT size)
    {
        if (size > size_)
        {
            size_ = size;
            value_ = std::make_unique<T[]>(size);
        }
    }

    void Reset()
    {
        value_.reset();
        size_ = 0;
    }

    UINT Size() const
    {
        return size_;
    }

    T* Get() const
    {
        return value_.get();
    }

    T* Get(UINT offset) const
    {
        return (value_.get() + offset);
    }

    template <class U>
    U* As() const
    {
        return reinterpret_cast<U*>(Get());
    }

    template <class U>
    U* As(UINT offset) const
    {
        return reinterpret_cast<U*>(Get(offset));
    }

    operator bool() const
    {
        return value_ != nullptr;
    }

    const T operator [](UINT index) const
    {
        if (index >= size_)
        {
            Debug::Error("Array index out of range: ", index, size_);
            return T(0);
        }
        return value_[index];
    }

    T& operator [](UINT index)
    {
        if (index >= size_)
        {
            Debug::Error("Array index out of range: ", index, size_);
            return value_[0];
        }
        return value_[index];
    }

private:
    std::unique_ptr<T[]> value_;
    UINT size_ = 0;
};
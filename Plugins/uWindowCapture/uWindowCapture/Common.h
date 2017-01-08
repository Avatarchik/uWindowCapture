#pragma once

#include <memory>
#include <functional>
#include <chrono>


// Unity interface and ID3D11Device getters
struct IUnityInterfaces;
IUnityInterfaces* GetUnity();

struct ID3D11Device;
ID3D11Device* GetDevice();


// Window manager getter
class WindowManager;
std::unique_ptr<WindowManager>& GetWindowManager();


// Window utility
bool IsAltTabWindow(HWND hWnd);


// Buffer
template <class T>
class Buffer
{
public:
    Buffer() {}
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


// Releaser
class ScopedReleaser
{
public:
    using ReleaseFuncType = std::function<void()>;
    ScopedReleaser(ReleaseFuncType&& func) : func_(func) {}
    ~ScopedReleaser() { func_(); }

private:
    const ReleaseFuncType func_;
};


// Timer
class ScopedTimer
{
public:
    using microseconds = std::chrono::microseconds;
    using TimerFuncType = std::function<void(microseconds)>;
    ScopedTimer(TimerFuncType&& func);
    ~ScopedTimer();

private:
    const TimerFuncType func_;
    const std::chrono::time_point<std::chrono::steady_clock> start_;
};

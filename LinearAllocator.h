#pragma once
#include <memory>
#include <cstddef>
#include <cassert>


struct LinearAllocatorBufferSpan
{
   explicit LinearAllocatorBufferSpan(std::uint8_t* ptr,
                                      std::size_t total);

   std::uint8_t* pBegin;
   std::uint8_t* pEnd;
   std::uint8_t* pCapacityEnd;
   std::size_t peak;
};


std::size_t CalculatePadding(std::size_t baseAddress, std::size_t alignment);


template<typename T>
class LinearAllocator
{
    template<typename U>
    friend class LinearAllocator;

public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    template< class U > struct rebind { using other = LinearAllocator<U>; };

    LinearAllocator() = default;
#ifdef _DEBUG
    ~LinearAllocator() { assert(externalAllocationsCounter_ == 0); }
#endif

    explicit LinearAllocator(LinearAllocatorBufferSpan* buffer) : buffer_(buffer) { assert(buffer != nullptr); }

    template<typename U>
    LinearAllocator(const LinearAllocator<U>& other) : buffer_(other.buffer_) { }

    template<typename U>
    LinearAllocator& operator=(const LinearAllocator<U>& other) { buffer_ = other.buffer_; }

    [[nodiscard]] T* allocate(std::size_t count)
    {
        const auto currentAddress = reinterpret_cast<std::size_t>(buffer_->pEnd);
        const std::size_t padding = CalculatePadding(currentAddress, __alignof(T));
        const std::size_t countBytes = count * sizeof(T);
        const std::size_t sumAlloc = padding + countBytes;
        buffer_->peak += sumAlloc;
        if (buffer_->pEnd + sumAlloc > buffer_->pCapacityEnd)
        {
#ifdef _DEBUG
            externalAllocationsCounter_++;
#endif
            return overflowAllocator_.allocate(count);
        }

        buffer_->pEnd += sumAlloc;
        return reinterpret_cast<T*>(currentAddress + padding);
    }

    void deallocate(T* p, std::size_t count)
    {
        auto ptr = reinterpret_cast<std::uint8_t*>(p);
        if(ptr >= buffer_->pBegin && ptr < buffer_->pCapacityEnd)
            return;

#ifdef _DEBUG
        externalAllocationsCounter_--;
#endif
        overflowAllocator_.deallocate(p, count);
    }

    [[nodiscard]] friend bool operator==(const LinearAllocator& lhs, const LinearAllocator& rhs) { return lhs.buffer_ == rhs.buffer_; }
    [[nodiscard]] friend bool operator!=(const LinearAllocator& lhs, const LinearAllocator& rhs) { return !(lhs == rhs); }

private:

#ifdef _DEBUG
    std::int64_t externalAllocationsCounter_ = 0;
#endif

    LinearAllocatorBufferSpan* buffer_ = nullptr;
    std::allocator<T> overflowAllocator_;
};

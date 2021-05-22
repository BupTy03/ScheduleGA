#include "LinearAllocator.h"


LinearAllocatorBufferSpan::LinearAllocatorBufferSpan(std::uint8_t* ptr, std::size_t total)
    : pBegin(ptr)
    , pEnd(ptr)
    , pCapacityEnd(ptr + total)
    , peak(0)
{
    assert(ptr != nullptr);
    assert(total > 0);
}


std::size_t CalculatePadding(std::size_t baseAddress, std::size_t alignment)
{
    if(baseAddress == 0 || alignment == 0)
        return 0;

    if(alignment > baseAddress)
        return alignment - baseAddress;

    return baseAddress % alignment;
}

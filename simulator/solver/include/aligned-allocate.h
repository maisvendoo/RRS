#ifndef     ALINGED_ALLOCATE_H
#define     ALINGED_ALLOCATE_H

#include    <new>

#if defined(_WIN32) || defined(_WIN64)

#include    <malloc.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
inline void *aligned_allocate(std::size_t size, std::size_t alignment)
{
    void *ptr = _aligned_malloc(size, alignment);

    if (!ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

inline void aligned_deallocate(void *ptr) noexcept
{
    _aligned_free(ptr);
}

#else

#include    <stdlib.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
inline void *aligned_allocate(std::size_t size, std::size_t alignment)
{
    void *ptr = nullptr;

    if (posix_memalign(&ptr, alignment, size) != 0 || !ptr)
    {
        throw std::bad_alloc();
    }

    return ptr;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
inline void aligned_deallocate(void *ptr) noexcept
{
    free(ptr);
}

#endif

#endif

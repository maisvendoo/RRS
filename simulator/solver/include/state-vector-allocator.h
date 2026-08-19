#ifndef     STATE_VECTOR_ALLOCATOR_H
#define     STATE_VECTOR_ALLOCATOR_H

#include    <cstddef>
#include    <aligned-allocate.h>

// Выравнивание по-умолчанию
constexpr size_t DEFAULT_ALIGNMENT = 64;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template<typename T, size_t Alignment = DEFAULT_ALIGNMENT>
class StateVectorAllocator
{
public:

    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;

    StateVectorAllocator() noexcept = default;

    template<typename U>
    StateVectorAllocator(const StateVectorAllocator<U> &) noexcept {}

    /// Выделение выровненной памяти
    [[nodiscard]] pointer allocate(size_type n)
    {
        if (n > std::size_t(-1) / sizeof(T))
        {
            throw std::bad_alloc();
        }

        return static_cast<pointer>(aligned_allocate(n * sizeof(T), Alignment));
    }

    /// Освобождение выровненной памяти
    void deallocate(pointer p, size_type) noexcept
    {
        aligned_deallocate(p);
    }

    template<typename U>
    struct rebind
    {
        using other = StateVectorAllocator<U, Alignment>;
    };
};

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template<typename T1, size_t A1, typename T2, size_t A2>
bool operator==(const StateVectorAllocator<T1, A1>&,
                const StateVectorAllocator<T2, A2>&) noexcept
{
    return true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template<typename T1, size_t A1, typename T2, size_t A2>
bool operator!=(const StateVectorAllocator<T1, A1>&,
                const StateVectorAllocator<T2, A2>&) noexcept
{
    return false;
}

#endif

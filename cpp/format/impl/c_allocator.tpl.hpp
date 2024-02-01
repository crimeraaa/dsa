#pragma once

#include <cstdlib> /* std::malloc, std::size_t */
#include <cstdio> /* std::perror */

template<class T>
struct C_Allocator {
    using value_type = T;
    using size_type = std::size_t;
    using pointer = value_type *;
    using const_pointer = const value_type *;

    static constexpr size_type max_size = static_cast<size_type>(-1); 
    static constexpr size_type max_allocs = max_size / sizeof(value_type);
    
    static pointer allocate(size_type n_count) noexcept
    {
        if (n_count == 0 || n_count >= SIZE_MAX / sizeof(T)) {
            return nullptr;    
        }
        void *p_memory = std::malloc(sizeof(value_type) * n_count);
        if (p_memory == nullptr) {
            std::perror("std::malloc() failed");
        }
        return static_cast<pointer>(p_memory);
    }

    /**
     * @brief   Frees your pointer but does not call destructors for objects.
     */ 
    static void deallocate(pointer p_memory, size_type n_count) noexcept
    {
        (void) n_count; //
        std::free(p_memory);
    }

    /**
     * @brief   Call your desired constructor for this item at this address.
     * 
     * @note    Because of the call to `new` this likely won't link with C.
     *          You'll need to link with `libstdc++` or something similar.
     */
    template<typename ...Args>
    static void construct(pointer p_item, Args ...ctor_args)
    {
        // Call global operator new to construct an already allocated object
        ::new (static_cast<void*>(p_item)) value_type(ctor_args...);    
    }

    static void destroy(pointer p_item)
    {
        p_item->~value_type; // This is valid even for typedefs of fundamentals.
    }
};

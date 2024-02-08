#ifndef TERRIBLE_ARRAY_HPP
#define TERRIBLE_ARRAY_HPP

#ifndef __cplusplus
#error "bruh"
#endif // __cplusplus

#include <memory>

template<class ElemT, class Alloc = std::allocator<ElemT>>
class Dyn_Array {
private:
    using value_type = ElemT;
    using size_type = std::size_t;
    using pointer = value_type*;
    using reference = value_type&;
    using const_pointer = const value_type*;
    using const_reference = const value_type&;

    using AllocTraits = std::allocator_traits<Alloc>;
private:
    pointer m_buffer{nullptr}; // Heap-allocated buffer
    size_type m_count{0}; // Number of elements currently in the buffer
    size_type m_capacity{0}; // Number of elements that `m_buffer` can hold.
public:
    Dyn_Array()
    {}

    Dyn_Array(size_type initial_size)
        : m_buffer{AllocTraits::allocate(initial_size)}
        , m_count{0}
        , m_capacity{initial_size}
    {}

    void push_back(const value_type &item)
    {
        if (m_count + 1 > m_capacity) {

        }
        m_buffer[m_count++] = item;
    }

    bool resize(size_type newsize)
    {
        if (newsize == m_capacity) {
            return true;
        }
        if (newsize < m_capacity) {
            m_count = newsize;
        } 
        m_capacity = newsize;
        pointer dummy = AllocTraits::allocate(m_capacity);
        for (size_t i = 0; i < m_count; i++) {
            void *where = static_cast<void*>(dummy + i);
            ::new (where) value_type(m_buffer[i]); // Placement new w/ ctor
        }
        AllocTraits::deallocate(m_buffer);
        m_buffer = dummy;
        return true;
    }
};

#endif // !TERRIBLE_ARRAY_HPP

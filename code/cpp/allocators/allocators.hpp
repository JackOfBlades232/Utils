#pragma once

#include <cassert>
#include <cstdlib>
#include <atomic>

// @TODO: stack
// @TODO: pool
// @TODO: freelist

// @TODO: mmap instead of malloc for allocator mem
// @TODO: memory orderings
// @TODO: concept

template <class T>
class LinearAllocator {
    T *m_arena = nullptr;
    std::atomic<T *> m_head{nullptr};
    size_t cap = 0;

public:
    LinearAllocator(size_t cap = 1 << 16)
        : m_arena((T *)malloc(cap * sizeof(T))), m_head(m_arena), cap(cap)
    {
        assert(m_arena && m_head.load() == m_arena);
    }
    ~LinearAllocator() { if (m_arena) free(m_arena); }

    [[nodiscard]] T *allocate(size_t n) {
        if (m_head.load() - m_arena > cap - n)
            return (T *)malloc(n * sizeof(T));
        else
            return m_head.fetch_add(n);
    }

    void deallocate(T *p, size_t n) noexcept {
        if (p < m_arena || p >= m_arena + cap) // Allocated with malloc
            free(p);
    }

    void reset() { m_head.store(m_arena); }
    size_t max_usage() const { return m_head.load() - m_arena; }

    inline static LinearAllocator &instance() {
        static LinearAllocator<T> inst;
        return inst;
    }
};

template <class T>
class StackAllocator {
    // @TODO

public:
    StackAllocator(size_t cap = 1 << 16);
    ~StackAllocator();

    [[nodiscard]] T *allocate(size_t n);
    void deallocate(T *p, size_t n) noexcept;
    void reset();
    size_t max_usage() const;

    inline static StackAllocator &instance() {
        static StackAllocator<T> inst;
        return inst;
    }
};

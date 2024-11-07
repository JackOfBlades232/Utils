#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <new>
#include <numeric>

// Semi-tested
// LinearAllocator -- single threaded
// LinearAllocatorMt -- mt, lock free

// @TODO: test properly
// StackAllocator -- stack allocator with out of order cleenup, single threaded
//     (out of order cleenup is too compicated, maybe even impossible with
//     lock free. Could be done w/ spinlock, but why bother)
// StackAllocatorMt -- stack allocator without out of order cleenup, lock free

// @TODO: pool
// @TODO: freelist

// @TODO: memory orderings
// @TODO: test with threading and tsan
// @TODO: refactor += -s

template <typename T, typename U>
inline constexpr size_t c_min_common_alignment =
    std::lcm(alignof(T), alignof(U));

inline constexpr size_t round_up_byte_size(size_t sz, size_t alignment)
{
    return alignment * ((sz - 1) / alignment + 1);
}

// Not mmap for portability
inline void *alloc_arena(size_t sz, size_t alignment)
{
    return (void *)(new (std::align_val_t{alignment}) char[sz]);
}

inline void free_arena(void *arena)
{
    delete[] (char *)arena;
}

template <class T>
class LinearAllocator {
    T *m_arena = nullptr;
    T *m_head = nullptr;
    size_t cap = 0;

public:
    LinearAllocator(size_t cap = 1 << 16)
        : m_arena((T *)alloc_arena(cap * sizeof(T), alignof(T))),
          m_head(m_arena), cap(cap)
    {
        assert(m_arena && m_head == m_arena);
    }
    ~LinearAllocator() { if (m_arena) free_arena(m_arena); }

    [[nodiscard]] T *allocate(size_t n) {
        if (m_head - m_arena > cap - n)
            return new T[n];
        else
            return m_head += n;
    }

    void deallocate(T *p, size_t) noexcept {
        if (p < m_arena || p >= m_arena + cap) // Allocated with dyn mem
            delete[] p;
    }

    void reset() { m_head = m_arena; }
    size_t max_usage() const { return (m_head - m_arena) * sizeof(T); }

    inline static LinearAllocator &instance() {
        static LinearAllocator<T> inst;
        return inst;
    }
};

template <class T>
class LinearAllocatorMt {
    T *m_arena = nullptr;
    std::atomic<T *> m_head{nullptr};
    size_t cap = 0;

public:
    LinearAllocatorMt(size_t cap = 1 << 16)
        : m_arena((T *)alloc_arena(cap * sizeof(T), alignof(T))),
          m_head(m_arena), cap(cap)
    {
        assert(m_arena && m_head.load() == m_arena);
    }
    ~LinearAllocatorMt() { if (m_arena) free_arena(m_arena); }

    [[nodiscard]] T *allocate(size_t n) {
        if (m_head.load() - m_arena > cap - n)
            return new T[n];
        else
            return m_head.fetch_add(n);
    }

    void deallocate(T *p, size_t) noexcept {
        if (p < m_arena || p >= m_arena + cap) // Allocated with dyn mem
            delete[] p;
    }

    void reset() { m_head.store(m_arena); }
    size_t max_usage() const { return (m_head.load() - m_arena) * sizeof(T); }

    inline static LinearAllocatorMt &instance() {
        static LinearAllocatorMt<T> inst;
        return inst;
    }
};

template <class T>
class StackAllocator {
    struct Header {
        uint64_t offset           : 63;
        uint64_t need_to_be_freed : 1;
    };

    char *m_arena = nullptr;
    char *m_head = nullptr;
    size_t cap = 0;

public:
    StackAllocator(size_t cap = 1 << 16)
        : m_arena((char *)alloc_arena(cap * sizeof(T), alignof(T))),
          m_head(m_arena), cap(cap)
    {
        assert(m_arena && m_head == m_arena);
    }
    ~StackAllocator() {
        if (m_arena)
            free_arena(m_arena);
    }

    [[nodiscard]] T *allocate(size_t n) {
        size_t byte_size    = n * sizeof(T);
        char *dest          = m_head;
        Header *next_header = (Header *)(dest + byte_size);

        m_head += byte_size + sizeof(Header);

        // @TODO: alignment concern
        next_header->offset           = byte_size;
        next_header->need_to_be_freed = false;

        printf("After push(%llu) := %llu\n", n, m_head - m_arena);

        return (T *)dest;
    }

    void deallocate(T *p, size_t n) noexcept {
        Header *my_header = (Header *)(p + n * sizeof(T));
        char *my_allocation_end = (char *)my_header + sizeof(Header);
        if (my_allocation_end < m_head) {
            my_header->need_to_be_freed = true;
            return;
        }

        m_head = (char *)p;

        while (m_head > m_arena) {
            Header *header = (Header *)(m_head - sizeof(Header));
            if (!header->need_to_be_freed)
                break;

            m_head -= header->offset + sizeof(Header);
        }

        printf("After pop(%llu) := %llu\n", n, m_head - m_arena);
    }

    void reset() { m_head = m_arena; }
    size_t max_usage() const { return m_head - m_arena; }

    inline static StackAllocator &instance() {
        static StackAllocator<T> inst;
        return inst;
    }
};

template <class T>
class StackAllocatorMt {
    static constexpr size_t c_block_alignment =
        c_min_common_alignment<T, uint64_t>;

    struct alignas(c_block_alignment) Header {
        std::atomic_uint64_t offset{0};
    };

    char *m_arena = nullptr;
    std::atomic<char *> m_head{nullptr};
    size_t cap = 0;

public:
    StackAllocatorMt(size_t cap = 1 << 16)
        : m_arena((char *)alloc_arena(cap * sizeof(T), c_block_alignment)),
          m_head(m_arena), cap(cap)
    {
        assert(m_arena && m_head.load() == m_arena);
        memset(m_arena, 0, cap * sizeof(T));
    }
    ~StackAllocatorMt() {
        if (m_arena)
            free_arena(m_arena);
    }

    [[nodiscard]] T *allocate(size_t n) {
        size_t byte_size = round_up_byte_size(n * sizeof(T), c_block_alignment);
        char *dest       = m_head.fetch_add(byte_size + c_block_alignment);
        Header *next_header = (Header *)(dest + byte_size);

        // @NOTE: this is technically UB and could work bad w/ TSAN
        //         (writing to non-contructed atomic)
        next_header->offset.store(byte_size);

        return (T *)dest;
    }

    void deallocate(T *p, size_t n) noexcept {
        auto header_from_head = [](char *head) {
            return (Header *)(head - c_block_alignment);
        };

        char *cur_head = m_head.load();
        Header *cur_header;
        size_t cur_offset;

        do {
            cur_header = header_from_head(cur_head);
            cur_offset = cur_header->offset.load();
            if ((uintptr_t)(cur_head - cur_offset) > (uintptr_t)p)
                return;
        } while (!m_head.compare_exchange_weak(
            cur_head, cur_head - cur_offset - c_block_alignment));
    }

    void reset() { m_head.store(m_arena); }
    size_t max_usage() const { return m_head.load() - m_arena; }

    inline static StackAllocatorMt &instance() {
        static StackAllocatorMt<T> inst;
        return inst;
    }
};

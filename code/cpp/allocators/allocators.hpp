#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <new>
#include <numeric>
#include <memory>
#include <utility>

// Semi-tested
// LinearAllocator -- single threaded
// LinearAllocatorMt -- mt, lock free
// StackAllocator -- stack allocator with out of order cleenup, single threaded
// StackAllocatorMt -- stack allocator without out of order cleenup, lock free
// PoolAllocator -- single threaded
// PoolAllocatorMt -- mt, lock free
// FreeListAllocator -- single threaded, with rudimentary coalescing
// FreeListAllocatorMt -- @TODO

// @NOTE: in LF implementaitons max_usage is incorrect, I don't want to track
//        another piece of state in a lock-free context
//
// I am also not sure that I've respected c++ lifetimes enough to eliminate
// all __real__ possibilities of UB-driven compiler optimizations.
// For example, in stack and FreeList allocators I did not perform destuctions
// on memory when repurposing.

// @TODO: test pool on single allocs
// @TODO: test mt (with tsan on linux)
// @TODO: memory orderings

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
    size_t m_cap = 0;

public:
    LinearAllocator(size_t cap = 1 << 16)
        : m_arena((T *)alloc_arena(cap * sizeof(T), alignof(T))),
          m_head(m_arena), m_cap(cap)
    {
        assert(m_arena && m_head == m_arena);
    }
    ~LinearAllocator() {
        if (m_arena) {
            reset();
            free_arena(m_arena);
        }
    }

    [[nodiscard]] T *allocate(size_t n) {
        if (m_head - m_arena > m_cap - n)
            return new T[n];
        else
            return m_head += n;
    }

    void deallocate(T *p, size_t) noexcept {
        if (p < m_arena || p >= m_arena + m_cap) // Allocated with dyn mem
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
    ~LinearAllocatorMt() { 
        if (m_arena) {
            reset();
            free_arena(m_arena);
        }
    }

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

    static constexpr size_t c_block_alignment =
        c_min_common_alignment<Header, T>;

    char *m_arena = nullptr;
    char *m_head = nullptr;
    size_t m_cap = 0;

#ifndef NDEBUG
    size_t max_head_offset = 0;
#endif

public:
    StackAllocator(size_t cap = 1 << 16)
        : m_arena((char *)alloc_arena(cap * sizeof(T), c_block_alignment)),
          m_head(m_arena), m_cap(cap)
    {
        assert(m_arena && m_head == m_arena);
    }
    ~StackAllocator() {
        if (m_arena) {
            reset();
            free_arena(m_arena);
        }
    }

    [[nodiscard]] T *allocate(size_t n) {
        size_t byte_size = round_up_byte_size(n * sizeof(T), c_block_alignment);
        char *dest       = m_head;
        Header *next_header = (Header *)(dest + byte_size);

        m_head += byte_size + c_block_alignment;

        if (m_head - m_arena > m_cap)
            return nullptr;

#ifndef NDEBUG
        if (size_t offset = m_head - m_arena; offset > max_head_offset)
            max_head_offset = offset;
#endif

        next_header->offset           = byte_size;
        next_header->need_to_be_freed = false;

        return (T *)dest;
    }

    void deallocate(T *p, size_t n) noexcept {
        Header *my_header =
            (Header *)((char *)p +
                       round_up_byte_size(n * sizeof(T), c_block_alignment));

        char *my_allocation_end = (char *)my_header + c_block_alignment;
        if (my_allocation_end < m_head) {
            my_header->need_to_be_freed = true;
            return;
        }

        m_head = (char *)p;

        while (m_head > m_arena) {
            Header *header = (Header *)(m_head - c_block_alignment);
            if (!header->need_to_be_freed)
                break;

            m_head -= header->offset + c_block_alignment;
        }
    }

    void reset() { m_head = m_arena; }
    size_t max_usage() const {
#ifndef NDEBUG
        return max_head_offset;
#else
        return m_head - m_arena;
#endif
    }

    inline static StackAllocator &instance() {
        static StackAllocator<T> inst;
        return inst;
    }
};

template <class T>
class StackAllocatorMt {
    T *m_arena = nullptr;
    std::atomic<T *> m_head{nullptr};
    size_t m_cap = 0;

public:
    StackAllocatorMt(size_t cap = 1 << 16)
        : m_arena((T *)alloc_arena(cap * sizeof(T), alignof(T))),
          m_head(m_arena), m_cap(cap)
    {
        assert(m_arena && m_head.load() == m_arena);
        memset(m_arena, 0, cap * sizeof(T));
    }
    ~StackAllocatorMt() {
        if (m_arena) {
            reset();
            free_arena(m_arena);
        }
    }

    [[nodiscard]] T *allocate(size_t n) {
        T* allocated = m_head.fetch_add(n * sizeof(T));

        if (m_head - m_arena > m_cap) {
            m_head.fetch_sub(n * sizeof(T));
            return nullptr;
        }

        return allocated;
    }

    void deallocate(T *p, size_t n) noexcept {
        T *cur_head = m_head.load();

        do {
            if ((uintptr_t)(cur_head) > (uintptr_t)(p + n * sizeof(T)))
                return;
        } while (!m_head.compare_exchange_weak(cur_head, p));
    }

    void reset() { m_head.store(m_arena); }
    size_t max_usage() const { return m_head.load() - m_arena; }

    inline static StackAllocatorMt &instance() {
        static StackAllocatorMt<T> inst;
        return inst;
    }
};

template <class T>
class PoolAllocator {
    union Block {
        T obj;
        Block *next;
    };

    Block *m_arena = nullptr;
    Block *m_head  = nullptr;
    size_t m_cap;

#ifndef NDEBUG
    size_t allocations = 0;
    size_t max_allocations = 0;
#endif

public:
    PoolAllocator(size_t cap = 1 << 16)
        : m_arena((Block *)alloc_arena(cap * sizeof(Block), alignof(Block))),
          m_head(m_arena), m_cap(cap)
    {
        assert(m_arena && m_head == m_arena);
        reset();
    }
    ~PoolAllocator() {
        if (m_arena) {
            reset();
            free_arena(m_arena);
        }
    }

    [[nodiscard]] T *allocate(size_t n = 1) {
        assert(n == 1);
        if (!m_head)
            return nullptr;

#ifndef NDEBUG
        ++allocations;
        if (allocations > max_allocations)
            max_allocations = allocations;
#endif

        return (T *)std::exchange(m_head, m_head->next);
    }
    void deallocate(T *p, size_t n = 1) noexcept {
        assert(n == 1);
        Block *b = (Block *)p;
        b->next = m_head;
        m_head = b;

#ifndef NDEBUG
        --allocations;
#endif
    }

    void reset() { 
        for (Block *p = m_arena; p < m_arena + (m_cap - 1); ++p)
            p->next = p + 1;
        m_arena[m_cap - 1].next = nullptr;
    }
    size_t max_usage() const {
#ifndef NDEBUG
        return max_allocations * sizeof(T);
#else
        return 0;
#endif
    }

    inline static PoolAllocator &instance() {
        static PoolAllocator<T> inst;
        return inst;
    }
};

template <class T>
class PoolAllocatorMt {
    union Block {
        T obj;
        std::atomic<Block *> next;

        Block() : next(nullptr) {}
    };

    Block *m_arena = nullptr;
    std::atomic<Block *> m_head{nullptr};
    size_t m_cap;

public:
    PoolAllocatorMt(size_t cap = 1 << 16)
        : m_arena((Block *)alloc_arena(cap * sizeof(Block), alignof(Block))),
          m_head(m_arena), m_cap(cap)
    {
        assert(m_arena && m_head.load() == m_arena);
        new (m_arena) Block[m_cap];
        for (Block *p = m_arena; p < m_arena + (m_cap - 1); ++p)
            p->next.store(p + 1);
    }
    ~PoolAllocatorMt() {
        if (m_arena) {
            reset();
            free_arena(m_arena);
        }
    }

    [[nodiscard]] T *allocate(size_t n = 1) {
        assert(n == 1);
        Block *new_head = nullptr;
        Block *old_head = m_head.load();
        
        do {
            if (!old_head)
                return nullptr;
            new_head = old_head->next.load();
        } while (!m_head.compare_exchange_weak(old_head, new_head));

        std::destroy_at((std::atomic<Block *> *)old_head);
        return old_head;
    }
    void deallocate(T *p, size_t n = 1) noexcept {
        assert(n == 1);
        new (p) Block{}; // Constructs atomic

        Block *new_head = (Block *)p;
        Block *old_head = m_head.load();

        do {
            new_head->next.store(old_head);
        } while (!m_head.compare_exchange_weak(old_head, new_head));
    }

    // @NOTE: can't be done concurrently w/ allocaitons/deallocations
    void reset() { 
        if (Block *p = m_head.load()) {
            Block *next = p;
            do {
                p = next;
                next = p->next.load();
                std::destroy_at((std::atomic<Block *> *)p);
            } while (next);
        }
        new (m_arena) Block[m_cap];
        for (Block *p = m_arena; p < m_arena + (m_cap - 1); ++p)
            p->next.store(p + 1);
    }
    size_t max_usage() const { return 0; }

    inline static PoolAllocatorMt &instance() {
        static PoolAllocatorMt<T> inst;
        return inst;
    }
};

template <class T>
class FreeListAllocator {
    struct FreeHeader {
        size_t size;
        FreeHeader *next;
    };

    union AllocatedHeader {
        size_t size;
        T unused__; // For alignment

        AllocatedHeader(size_t sz) : size(sz) {}
    };

    static constexpr size_t c_block_alignment =
        c_min_common_alignment<FreeHeader, AllocatedHeader>;

    char *m_arena = nullptr;
    FreeHeader *m_head  = nullptr;
    size_t m_cap;

#ifndef NDEBUG
    size_t allocated_blocks = 0;
    size_t max_allocated_blocks = 0;
#endif

public:
    FreeListAllocator(size_t cap = 1 << 16)
        : m_arena((char *)alloc_arena(cap * sizeof(T), c_block_alignment)),
          m_head((FreeHeader *)m_arena), m_cap(cap)
    {
        assert(m_arena && (char *)m_head == m_arena);
        new (m_arena) FreeHeader{m_cap / c_block_alignment, nullptr};
    }
    ~FreeListAllocator() {
        if (m_arena) {
            reset();
            free_arena(m_arena);
        }
    }

    [[nodiscard]] T *allocate(size_t n) {
        FreeHeader *free = m_head, *prev = nullptr;
        size_t byte_size = round_up_byte_size(
            sizeof(AllocatedHeader) + n * sizeof(T), c_block_alignment);
        size_t min_blocks = byte_size / c_block_alignment;

        while (free) {
            size_t free_size = free->size;
            FreeHeader *next = free->next;
            if (free_size >= min_blocks) {

                AllocatedHeader *alloc = (AllocatedHeader *)free;
                new (alloc) AllocatedHeader{min_blocks};

                if (free_size > min_blocks) {
                    FreeHeader *new_header =
                        (FreeHeader *)((char *)free + byte_size);
                    new (new_header) FreeHeader{free_size - min_blocks, next};
                    next = new_header;
                }

               if (prev)
                   prev->next = next;
               else
                   m_head = next;

#ifndef NDEBUG
               allocated_blocks += min_blocks;
               if (allocated_blocks > max_allocated_blocks)
                   max_allocated_blocks = allocated_blocks;
#endif

               return (T *)(alloc + 1);
            }
        }

        return nullptr;
    }

    void deallocate(T *p, size_t) noexcept {
        AllocatedHeader *alloc = (AllocatedHeader *)p - 1;
        size_t size = alloc->size;

#ifndef NDEBUG
        allocated_blocks -= alloc->size;
#endif

        FreeHeader *free = (FreeHeader *)alloc;
        
        // If next to each other, coalesce
        while (m_head ==
               (FreeHeader *)((char *)free + size * c_block_alignment))
        {
            size += m_head->size;
            FreeHeader *next = m_head->next;
            m_head = next;
        }

        new (free) FreeHeader{size, m_head};
        m_head = free;
    }

    // @NOTE: defrag could go over the free list and coalesce out of order

    void reset() {
        m_head = (FreeHeader *)m_arena;
        new (m_head) FreeHeader{m_cap / c_block_alignment, nullptr};
    }
    size_t max_usage() const {
#ifndef NDEBUG
        return max_allocated_blocks * c_block_alignment;
#else
        return 0;
#endif
    }

    inline static FreeListAllocator &instance() {
        static FreeListAllocator<T> inst;
        return inst;
    }
};


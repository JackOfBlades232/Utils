#include <cstdio>
#include <vector>

#include "allocators.hpp"

// c++ abstraction for implament stateless stl allocator
template <template <class> class Allocator, class T>
struct AllocatorProxy {
    typedef T value_type;
    Allocator<T> &allocator_implementation;
    AllocatorProxy() : allocator_implementation(Allocator<T>::instance()) {}

    [[nodiscard]] T *allocate(std::size_t n) {
        return allocator_implementation.allocate(n);
    }

    void deallocate(T *p, std::size_t n) noexcept {
        return allocator_implementation.deallocate(p, n);
    }
    static void reset() { Allocator<T>::instance().reset(); }
    static std::size_t max_usage() {
        return Allocator<T>::instance().max_usage();
    }
    template <typename _Tp1>
    struct rebind {
        typedef AllocatorProxy<Allocator, _Tp1> other;
    };
};

template <typename T>
using ExaminatedAllocator = AllocatorProxy<StackAllocator, T>;
// here you can change LinearAllocator to StackAllocator, PoolAllocator of
// FreeListAllocator

int main()
{
    {
        std::vector<int, ExaminatedAllocator<int>> x(16);
        std::vector<int, ExaminatedAllocator<int>> y(32);
        std::vector<int, ExaminatedAllocator<int>> z(64);
    }
    printf("Right Bracket Sequences Max Memory Usage %lld\n",
           ExaminatedAllocator<int>::max_usage());
    ExaminatedAllocator<int>::reset(); // reset allocator after test

    {
        std::vector<int, ExaminatedAllocator<int>> x(16);
        std::vector<int, ExaminatedAllocator<int>> y(32);
        std::vector<int, ExaminatedAllocator<int>> z(64);
        x.clear();
        x.shrink_to_fit();
        y.clear();
        y.shrink_to_fit();
        z.clear();
        z.shrink_to_fit();
        /*
        x = {};
        y = {};
        z = {};
        */
    }
    printf("Wrong Bracket Sequences Max Memory Usage %lld\n",
           ExaminatedAllocator<int>::max_usage());
    ExaminatedAllocator<int>::reset(); // reset allocator after test

    /*
    {
        for (int i = 0; i < 128; i++) {
            std::vector<int, ExaminatedAllocator<int>> x(64);
        }
    }
    */
    printf("Allocator memory reusing Max Memory Usage %lld\n",
           ExaminatedAllocator<int>::max_usage());
    ExaminatedAllocator<int>::reset(); // reset allocator after test
}

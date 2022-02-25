#pragma once
#include <cstdint>
#include <vector>

namespace python::gc {

class Allocator {
public:
    void* alloc(std::size_t size) {
        void* ptr = ::operator new(size);
        if (size > k_limit) {
            m_large_objects.push_back(ptr);
        }
        else {
            m_small_objects.push_back(ptr);
        }
        return ptr;
    }

    void dealloc(void* ptr, std::size_t size) {
        m_unordered_objects.emplace_back(ptr, size);
    }

private:
    static constexpr std::size_t k_limit = 128;

    std::vector<void*> m_small_objects;
    std::vector<void*> m_large_objects;
    std::vector<std::pair<void*, std::size_t>> m_unordered_objects;
};


}

namespace python {

inline gc::Allocator g_gc;

}
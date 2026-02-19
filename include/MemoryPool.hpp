#pragma once

#include "Order.hpp"

#include <cstddef>
#include <new>
#include <stdexcept>
#include <utility>

class MemoryPool
{
public:
    // Generates the overall memory pool, forming a linked list of 
    explicit MemoryPool(size_t size)
        : poolSize_{size}
    {
        // Allocate per inputted size via placement new
        pool_ = static_cast<Order*>(::operator new(size * sizeof(Order)));

        for (size_t i = 0; i < poolSize_ - 1; ++i)
        {
            pool_[i].next_ = &pool_[i + 1];
        }

        pool_[poolSize_ - 1].next_ = nullptr; // Last order in pool points to null

        firstFree_ = pool_;
    }

    ~MemoryPool()
    {
        ::operator delete(pool_);
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    // Parameter pack grabs standard construction parameters for Order
    template <typename... Args>
    Order* allocate(Args&&... args)
    {
        if (!firstFree_)
        {
            throw std::runtime_error("Memory pool exhausted");
        }

        // Shift the memory pool's first pointer
        Order* order = firstFree_;
        firstFree_ = firstFree_->next_;

        return new (order) Order(std::forward<Args>(args)...);
    }

    void deallocate(Order* order)
    {
        order->next_ = firstFree_;
        firstFree_ = order;
    }

private:
    size_t poolSize_;
    Order* pool_;      // Raw pointer to first Order object in memory pool
    Order* firstFree_; // Continuously managed pointer to first Order object in pool
};
#pragma once

#include "Order.hpp"

#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <stdexcept>
#include <utility>

class MemoryPool
{
public:
    // Generates the overall memory pool, forming a linked list of slots,
    // carrying next pointers in raw, unconstructed memory
    explicit MemoryPool(std::size_t size)
        : poolSize_{size}
    {
        // Allocate memory pool per inputted size
        if (size == 0)
        {
            throw std::logic_error("Can't allocate pool of size 0");
        }

        pool_ = static_cast<MemorySlot*>(::operator new(size * sizeof(MemorySlot)));

        // Assign next pointers in raw memory
        for (std::size_t i = 0; i < poolSize_ - 1; ++i)
        {
            pool_[i].next_ = &pool_[i + 1];
        }

        // Assign first and last pointers
        pool_[poolSize_ - 1].next_ = nullptr; // Last slot in pool points to null
        firstFree_ = pool_;
    }


    ~MemoryPool()
    {
        ::operator delete(pool_);
    }

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;

    Order* allocate(OrderID id, Price price, Quantity quantity, Side side, TimeInForce tif=TimeInForce::GTC)
    {
        if (!firstFree_)
        {
            throw std::runtime_error("Memory pool exhausted");
        }

        // Shift the memory pool's first pointer
        MemorySlot* firstSlot = firstFree_;
        firstFree_ = firstFree_->next_;
        
        Order* newOrder = new (static_cast<void*>(&firstSlot->buffer_)) Order(id, price, quantity, side, tif);

        return newOrder;
    }

    void deallocate(Order* order)
    {
        // Destruct at the order's location
        std::destroy_at(order);

        // Order to return has the same base address as its parent slot
        MemorySlot* returnedSlot = reinterpret_cast<MemorySlot*>(order);
        static_assert(offsetof(MemorySlot, buffer_) == 0);

        // Return slot to memory pool
        returnedSlot->next_ = firstFree_;
        firstFree_ = returnedSlot;
    }

private:
    struct MemorySlot
    {
        alignas(Order) std::byte buffer_[sizeof(Order)];
        MemorySlot* next_;
    };

    std::size_t poolSize_;
    MemorySlot* pool_;      // Raw pointer to first Order object in memory pool
    MemorySlot* firstFree_; // Continuously managed pointer to first Order object in pool
};
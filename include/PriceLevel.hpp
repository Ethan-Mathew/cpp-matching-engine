#pragma once

#include "Order.hpp"
#include "Types.hpp"

#include <algorithm>
#include <cstdint>

class PriceLevel
{
public:
    struct MatchResult
    {
        Quantity executedQuantity;
        Order* filledOrderPtr  = nullptr;
        OrderID filledOrderID;
        bool orderFilled       = false;
    };

    explicit PriceLevel(Price price)
        : price_{price}
    {
    }

    void addOrder(Order* order)
    {
        // Internal accounting modified by new order 
        totalVolume_ += order->quantity_;
        orderCount_++;

        order->parentLevel_ = this;
        
        if (!head_) // If linked-list is empty
        {
            head_ = order;
            tail_ = order;
            order->prev_ = nullptr;
            order->next_ = nullptr;
        }
        else // Add to back of linked-list
        {
            order->prev_ = tail_;
            order->next_ = nullptr;
            tail_->next_ = order;
            tail_ = order;
        }
    }

    void removeOrder(Order* order)
    {
        totalVolume_ -= order->quantity_;
        orderCount_--;

        // Handle case where removed order is the linked-list head
        if (order->prev_)
        {
            (order->prev_)->next_ = order->next_;
        }
        else
        {
            head_ = order->next_;
        }

        // Handle case where removed order is the linked-list tail
        if (order->next_)
        {
            (order->next_)->prev_ = order->prev_;
        }
        else
        {
            tail_ = order->prev_;
        }

        order->prev_ = nullptr;
        order->next_ = nullptr;
    }

    // Execute matches for the current price level given a quantity
    MatchResult executeMatchStep(Quantity incomingQuantity)
    {
        Order* target = head_;
        // Trade based on the first order in the price level queue
        Quantity tradeableQuantity = std::min(incomingQuantity, head_->quantity_);
        
        target->quantity_ -= tradeableQuantity;
        totalVolume_ -= tradeableQuantity;

        MatchResult tradeResult{tradeableQuantity};

        if (target->quantity_ == 0)
        {
            tradeResult.orderFilled = true;
            tradeResult.filledOrderID = target->id_;
            tradeResult.filledOrderPtr = target;
            removeOrder(target);
        }

        return tradeResult;
    }

    [[nodiscard]] Price getPrice() const { return price_; }
    [[nodiscard]] uint64_t getTotalVolume() const { return totalVolume_; }
    [[nodiscard]] uint32_t getOrderCount() const { return orderCount_; }
    [[nodiscard]] const Order* getHead() const { return head_; }
    [[nodiscard]] const Order* getTail() const { return tail_; }

private:
    Price price_;
    uint64_t totalVolume_ = 0;
    uint32_t orderCount_  = 0;

    Order* head_ = nullptr;
    Order* tail_ = nullptr;
};
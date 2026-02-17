#pragma once

#include "Types.hpp"

struct Order
{
    OrderID id_;
    Price price_;
    Order* next_ = nullptr;
    Order* prev_ = nullptr;
    Quantity quantity_;
    Side side_;

    Order() = delete;

    Order(OrderID id, Price price, Quantity quantity, Side side)
        : id_{id}
        , price_{price}
        , quantity_{quantity}
        , side_{side}
    {
    }
};
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
    TimeInForce tif_;

    Order() = delete;

    Order(OrderID id, Price price, Quantity quantity, Side side, TimeInForce tif = TimeInForce::GTC)
        : id_{id}
        , price_{price}
        , quantity_{quantity}
        , side_{side}
        , tif_{tif}
    {
    }
};
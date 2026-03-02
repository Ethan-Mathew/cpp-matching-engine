#pragma once

#include "MemoryPool.hpp"
#include "Order.hpp"
#include "PriceLevel.hpp"
#include "Types.hpp"

#include <functional>
#include <map>
#include <unordered_map>

class OrderBook
{
public:
    explicit OrderBook(MemoryPool& pool)
        : pool_{pool}
    {
    }

    ~OrderBook()
    {
        for (auto& [price, level] : bidLevels_)
        {
            delete level;
        }

        for (auto [price, level] : askLevels_)
        {
            delete level;
        }
    }

    void addLimitOrder(OrderID id, Price price, Quantity qty, Side side, TimeInForce tif)
    {
        Order* newLimitOrder = pool_.allocate(id, price, qty, side, tif);
        
        auto [it, inserted] = orders_.emplace(id, newLimitOrder);
        if (!inserted)
        {
            pool_.deallocate(newLimitOrder);
            return;
        }

        getOrCreateLevel(price, side)->addOrder(newLimitOrder);
    }

    //void matchOrder(Order* order)

    void cancelOrder(OrderID id)
    {
        auto orderIt = orders_.find(id);
        if (orderIt == orders_.end()) return;

        Order* order = orderIt->second;
        PriceLevel* level = nullptr;

        // Explicitly find the level based on side
        if (order->side_ == Side::Buy)
        {
            auto it = bidLevels_.find(order->price_);
            if (it != bidLevels_.end()) level = it->second;
        }
        else
        {
            auto it = askLevels_.find(order->price_);
            if (it != askLevels_.end()) level = it->second;
        }

        if (level)
        {
            level->removeOrder(order);
        }

        orders_.erase(orderIt);
        pool_.deallocate(order);
    }

private:
    PriceLevel* getOrCreateLevel(Price price, Side side)
    {
        if (side == Side::Buy)
        {
            auto levelItr = bidLevels_.find(price);
            if (levelItr == bidLevels_.end())
            {
                PriceLevel* newPriceLevel = new PriceLevel(price);
                bidLevels_.emplace(price, newPriceLevel);
                return newPriceLevel;
            }
            else
            {
                return levelItr->second;
            }
        }
        else if (side == Side::Sell)
        {
            auto levelItr = askLevels_.find(price);
            if (levelItr == askLevels_.end())
            {
                PriceLevel* newPriceLevel = new PriceLevel(price);
                askLevels_.emplace(price, newPriceLevel);
                return newPriceLevel;
            }
            else
            {
                return levelItr->second;
            }
        }
    }

    std::map<Price, PriceLevel*, std::greater<Price>> bidLevels_;
    std::map<Price, PriceLevel*, std::less<Price>> askLevels_;
    std::unordered_map<OrderID, Order*> orders_;
    MemoryPool& pool_;
};
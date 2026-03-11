#pragma once

#include "MemoryPool.hpp"
#include "Order.hpp"
#include "PriceLevel.hpp"
#include "Types.hpp"

#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
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

        for (auto& [price, level] : askLevels_)
        {
            delete level;
        }
    }

    void addLimitOrder(OrderID id, Price price, Quantity qty, Side side, TimeInForce tif)
    {
        // Verify order ID is not a duplicate
        if (orders_.find(id) != orders_.end())
        {
            throw std::runtime_error("Duplicate order entered");
        }

        // Try to create the new order if memory is available
        Order* newLimitOrder;
        try
        {
            newLimitOrder = pool_.allocate(id, price, qty, side, tif);
        }
        catch(const std::exception&)
        {
            std::cerr << "Allocation failed for order " << id << '\n';
            throw;
        }

        matchOrder(newLimitOrder);

        // If the new order does not complete matching and is still alive
        if (newLimitOrder->quantity_ > 0)
        {
            auto [it, inserted] = orders_.emplace(id, newLimitOrder);
            if (!inserted)
            {
                pool_.deallocate(newLimitOrder);
                return;
            }

            getOrCreateLevel(price, side)->addOrder(newLimitOrder);
        }
        else
        {
            pool_.deallocate(newLimitOrder);
        }
    }

    void matchOrder(Order* newOrder)
    {
        // Buy orders deduct from ask side
        if (newOrder->side_ == Side::Buy)
        {
            while (newOrder->quantity_ > 0 && !askLevels_.empty())
            {
                // Find the best price level
                auto matchingLevelItr = askLevels_.begin();
                PriceLevel* matchingLevel = matchingLevelItr->second;

                // If we cannot find a suitable pricelevel for the incoming order
                if (newOrder->price_ < matchingLevel->getPrice())
                {
                    break;
                }
                
                // Execute the trade on the best matching price level
                PriceLevel::MatchResult executionResult = matchingLevel->executeMatchStep(newOrder->quantity_);
                newOrder->quantity_ -= executionResult.executedQuantity;

                std::cout << "> TRADE EXECUTED: " << executionResult.executedQuantity 
                          << " shares @ $" << matchingLevel->getPrice() << "\n";

                if (executionResult.orderFilled)
                {
                    orders_.erase(executionResult.filledOrderID);
                    pool_.deallocate(executionResult.filledOrderPtr);
                }
                
                if (matchingLevel->isEmpty())
                {
                    delete matchingLevel;
                    askLevels_.erase(matchingLevelItr);
                }
            }
        }
        else
        {
            while (newOrder->quantity_ > 0 && !bidLevels_.empty())
            {
                auto matchingLevelItr = bidLevels_.begin();
                PriceLevel* matchingLevel = matchingLevelItr->second;

                // If we cannot find a suitable pricelevel for the incoming order
                if (newOrder->price_ > matchingLevel->getPrice())
                {
                    break;
                }
                
                // Execute the trade on the best matching price level
                PriceLevel::MatchResult executionResult = matchingLevel->executeMatchStep(newOrder->quantity_);
                newOrder->quantity_ -= executionResult.executedQuantity;

                std::cout << "> TRADE EXECUTED: " << executionResult.executedQuantity 
                          << " shares @ $" << matchingLevel->getPrice() << "\n";

                if (executionResult.orderFilled)
                {
                    orders_.erase(executionResult.filledOrderID);
                    pool_.deallocate(executionResult.filledOrderPtr);
                }
                
                if (matchingLevel->isEmpty())
                {
                    delete matchingLevel;
                    bidLevels_.erase(matchingLevelItr);
                }
            }
        }
    }

    void cancelOrder(OrderID id)
    {
        auto orderIt = orders_.find(id);

        // Don't do anything if order ID is invalid
        if (orderIt == orders_.end()) return;

        Order* order = orderIt->second;

        // Remove the order from its price level
        PriceLevel* level = order->parentLevel_;
        level->removeOrder(order);
        
        // If the price level reaches zero orders after order cancellation, remove that level
        if (level->isEmpty())
        {
            if (order->side_ == Side::Buy)
            {
                bidLevels_.erase(order->price_);
            }
            else if (order->side_ == Side::Sell)
            {
                askLevels_.erase(order->price_);
            }

            delete order->parentLevel_;
        }

        // Remove the order from the orders hashmap and free its memory
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

        return nullptr;
    }

    using BidLevels = std::map<Price, PriceLevel*, std::greater<Price>>;
    using AskLevels = std::map<Price, PriceLevel*, std::less<Price>>;
    using OrderMap = std::unordered_map<OrderID, Order*>;

    MemoryPool& pool_;
    BidLevels bidLevels_;
    AskLevels askLevels_;
    OrderMap orders_;
};
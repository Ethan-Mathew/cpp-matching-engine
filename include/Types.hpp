#pragma once

#include <cstdint>

enum class Side : uint8_t
{
    Buy,
    Sell
};

// Preferring int64_t to double for pricing numerical stability
// Scaled by factor of 10000 (e.g. 1.2345 = 12345)
using Price     = int64_t;
using Quantity  = uint32_t;
using OrderID   = uint64_t;
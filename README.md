# C++ Matching Engine Version 1.0

A first attempt at building a limit order book with basic order matching and memory management features.

## Features

- Supports **FIFO** order matching algorithm using dedicated **price-level linked-lists** for each traded price.
- Custom memory management using a **memory pool** pre-allocation scheme.
- Implemented **O(1)** add and remove operations for orders.

## Improvements for Version 2.0

- Implement **Time in Force** (TIF is present in this implementation, though its functionality not fleshed-out).
- Enable **thread safety** for concurrent accesses.
- Add comprehensive **unit test** suite as well as performance **benchmarking**.
- Find a solution for the current usage of **std::map**, implementing true **O(1)** operations versus **O(logN)**.

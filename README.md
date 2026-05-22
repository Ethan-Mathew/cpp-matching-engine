# C++ Matching Engine Version 1.0

A first attempt at building a limit order book with basic order matching and memory management features. The order book features implemented were meant to build my familiarity with some market microstructure concepts, data routing, efficient memory management, and clean type/interface design.

## Features

- Supports **FIFO** order matching algorithm using dedicated **price-level linked-lists** for each traded price.
- Custom memory management using a **memory pool** pre-allocation scheme.
- Implemented **O(1)** add and remove operations for orders.

## Improvements for Version 2.0

- Implement **Time in Force** (TIF is present in this implementation, though its functionality not fleshed-out).
- Add comprehensive **unit test** suite as well as performance **benchmarking**.
- Find a solution for the current usage of **std::map**, implementing true **O(1)** operations versus **O(logN)**.
  - Circumvent improvements in big-O time complexity using structures catered to market behaviour of **specific assets**, preferring **cache availability** for performance assurances over pure time complexity.
- Enable compatibility with **real market data streams** (e.g. FIX, NASDAQ ITCH/OUCH 5.0, CME Datamine).
- More efficient logging, eliminating **std::cout** from the program hot-path with an **asynchronous** logging solution for verifiable order book **replay** and **reconstruction**.
  - Enable **data querying** including **L1, L2, L3** depth information retrieval.

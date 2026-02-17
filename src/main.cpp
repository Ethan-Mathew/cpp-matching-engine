#include "../include/Order.hpp"

#include <iostream>

int main()
{
    // Buy order for 100 at $150
    Order myOrder{1, 1500000, 100, Side::Buy};

    std::cout << "Order successfully created!" << std::endl;
    std::cout << "ID: " << myOrder.id_ << std::endl;
    std::cout << "Price: " << myOrder.price_ << std::endl;
    
    return 0;
}
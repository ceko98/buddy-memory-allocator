#include "../include/Allocator.h"
#include <iostream>

int main()
{
    std::cout << sizeof(Allocator) << std::endl;

    // const Allocator *alloc = Allocator::get_instance();
    return 0;
}
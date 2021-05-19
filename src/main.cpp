#include "../include/Allocator.h"
#include <iostream>

int main()
{
    Allocator *alloc = Allocator::get_instance();
    std::cout << "end init" << std::endl;
    
    return 0;
}
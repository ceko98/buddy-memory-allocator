#include "../include/Allocator.h"
#include <iostream>

int main()
{
    std::cout << sizeof(size_t) << std::endl;

    Allocator *alloc = Allocator::get_instance();
    std::cout << "end init" << std::endl;
    // alloc->profile();

    // char * a = (char *)malloc(1000);
    // std::cout << (1<<9) << std::endl;
    // std::cout << a + 1 << std::endl;
    
    return 0;
}
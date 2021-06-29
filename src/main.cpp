#include "../include/Allocator.h"
#include <iostream>

class A
{
private:
    int a[50];
public:
    A() {};
};

void* operator new(std::size_t sz) {
    return Allocator::get_instance()->allocate(sz);
}

void operator delete(void * ptr) {
    Allocator::get_instance()->free(ptr);
}

int main()
{
    A * a = new A();
    delete a;
    return 0;
}
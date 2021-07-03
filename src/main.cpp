#include "../include/Allocator.h"
#include <iostream>
#include <thread>
#include <mutex>

using namespace std::chrono_literals;

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

void allocate()
{

    int * a = new int;
    std::this_thread::sleep_for(5ms);
    delete a;
}

int main()
{
    std::thread t1(allocate);
    std::thread t2(allocate);
    std::thread t3(allocate);

    t1.join();
    t2.join();
    t3.join();
    // A * a = new A();
    // delete a;
    return 0;
}
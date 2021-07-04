/*
Simple tests for the memory allocator
These will use the default 1024B total space and 32B leaf
*/

#include "Allocator.h"
#include <iostream>
#include <thread>
#include <mutex>

#ifndef __TESTS__
#define __TESTS__

void *operator new(size_t sz)
{
    return Allocator::get_instance(true)->allocate(sz);
}

void operator delete(void *ptr)
{
    Allocator::get_instance(true)->free(ptr);
}

void allocate_and_free_some_variables()
{
    int *a = new int;
    char *b = new char;
    double *c = new double;
    size_t *d = new size_t;

    delete a;
    delete b;
    delete c;
    delete d;
}

void allocate_larger_structures()
{
    struct A
    {
        int arr[15]; // 15 * 4B = 60B
    };
    A *a = new A;
    delete a;
}

void allocate_all_available_space()
{
    /*
    Booking data is the data of the lists + the data for slit and alloc bit maps
    For 1024B total and leaf size 32B this is:
    levels * 8B + 2 * (all_blocks / 8) = 48B
    which will take 2 leaves (total 64B)
    so we have a whole 960B
    */
    int *arr[32];
    for (int i = 0; i < 32; i++)
    {
        arr[i] = new int;
    }
    for (int i = 0; i < 32; i++)
    {
        delete arr[i];
    }
}

void allocate()
{
    int * a = new int;
    delete a;
};
void allocate_delay()
{
    using namespace std::chrono_literals;
    int * a = new int;
    std::this_thread::sleep_for(5ms);
    delete a;
};
void behaves_correctly_in_multithreaded_enviroment()
{
    std::thread t1(allocate_delay);
    std::thread t2(allocate);
    std::thread t3(allocate);

    t1.join();
    t2.join();
    t3.join();
}

void run_tests()
{
    allocate_and_free_some_variables();
    std::cout << "--------------------------------" << std::endl; 
    allocate_larger_structures();
    std::cout << "--------------------------------" << std::endl; 
    allocate_all_available_space();
    std::cout << "--------------------------------" << std::endl;
    behaves_correctly_in_multithreaded_enviroment();
}

#endif
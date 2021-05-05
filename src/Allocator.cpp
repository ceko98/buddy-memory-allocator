#include <cstdlib>
#include <exception>
#include <cmath>
#include "../include/Allocator.h"

Allocator *Allocator::instance = nullptr;
size_t Allocator::LEVELS_COUNT = log2(INITIAL_SIZE / MIN_BLOCK_SIZE);

Allocator::Allocator() : heap_beg(nullptr), heap_size(0)
{
    heap_beg = malloc(INITIAL_SIZE);
    heap_size = INITIAL_SIZE;

    if (heap_beg == nullptr)
    {
        throw std::bad_alloc();
    }
}

Allocator *Allocator::get_instance()
{
    if (instance == nullptr)
    {
        instance = new Allocator();
    }

    return instance;
}

void Allocator::init_lists()
{
    lists = (LevelListNode **)heap_beg;

    for (size_t i = 0; i < LEVELS_COUNT; i++)
    {
        lists[i] = nullptr;
    }

    bitmap = lists[LEVELS_COUNT];
    bitmap_size = (1<<LEVELS_COUNT);
}

void *Allocator::allocate(size_t size)
{
}
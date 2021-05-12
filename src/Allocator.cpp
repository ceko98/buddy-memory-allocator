#include <cstdlib>
#include <exception>
#include <cmath>
#include <iostream>
#include "../include/Allocator.h"

int lower_pow_2(size_t num)
{
    return pow(2, (int)log2(num));
}

size_t compute_pow_2(size_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;

    return v;
}

size_t Allocator::size_of_level(int n)
{
    return heap_size / (1 << n);
}

Allocator *Allocator::instance = nullptr;
size_t Allocator::LEVELS_COUNT = log2(HEAP_SIZE / LEAF_SIZE);

Allocator::Allocator() : heap_beg(nullptr), heap_size(0)
{
    heap_beg = static_cast<char *>(malloc(HEAP_SIZE));
    heap_size = HEAP_SIZE;

    if (heap_beg == nullptr)
    {
        throw std::bad_alloc();
    }

    init_metadata();
}

Allocator *Allocator::get_instance()
{
    if (instance == nullptr)
    {
        instance = new Allocator();
    }

    return instance;
}

void Allocator::init_metadata()
{
    lists = (LevelListPointer *)heap_beg;
    lists_size = LEVELS_COUNT * sizeof(LevelListPointer *);
    for (int i = 0; i < LEVELS_COUNT; i++)
    {
        lists[i] = nullptr;
    }

    const size_t num_of_blocks = 1 << LEVELS_COUNT;
    allocated_map = heap_beg + lists_size;
    allocated_map_size = num_of_blocks / 8 + (num_of_blocks % 8 == 1);
    split_map = allocated_map + allocated_map_size;
    split_map_size = num_of_blocks / 8 + (num_of_blocks % 8 == 1);

    size_t metadata_size = lists_size + allocated_map_size + split_map_size;

    for (int i = LEVELS_COUNT - 1; i >= 0; i--)
    {
        size_t level_size = size_of_level(i);
        char *index_ptr = heap_beg;
        while (index_ptr < heap_beg + metadata_size)
        {
            int block_index = index_in_level(index_ptr, i);
            set_allocation_map_bit_at(block_index, true);
            set_split_map_bit_at(block_index, true);
            index_ptr += level_size;
        }

        if (i != 0 && block_index_on_level(index_ptr, i) % 2 == 1)
        {
            lists[i] = (LevelListPointer)index_ptr;
            lists[i]->next = nullptr;
        }
    }
}

void *Allocator::allocate(size_t size)
{
    size_t return_block_size = compute_pow_2(size);
    int block_level = block_size_to_level(return_block_size);

    void *block = get_block(block_level);
    return get_block(block_level);
}

void *Allocator::get_block(int level)
{
    if (lists[level] == nullptr)
    {
        split_blocks(level - 1);
    }

    LevelListNode *block = lists[level];
    lists[level] = block->next;
    return block;
}

void Allocator::split_blocks(int level)
{
    if (level > 0 && lists[level] == nullptr)
    {
        split_blocks(level - 1);
    }
    LevelListNode *current_level_first_box = lists[level];
    lists[level] = current_level_first_box->next;

    lists[level + 1] = current_level_first_box;
    current_level_first_box->next = current_level_first_box + size_of_level(level + 1);
    current_level_first_box->next->next = nullptr;
}

int Allocator::block_index_on_level(char *ptr, int level)
{
    return ((char *)ptr - (char *)heap_beg) / size_of_level(level);
}

int Allocator::block_size_to_level(size_t size)
{
    return LEVELS_COUNT - floor(log2(size / LEAF_SIZE));
}

int Allocator::index_in_level(char *ptr, int level)
{
    return (ptr - heap_beg) / size_of_level(level) + (1 << level) - 1;
}

void Allocator::set_split_map_bit_at(int index, bool to_alloc)
{
    if (index >= (1 << LEVELS_COUNT) - 1)
    {
        return;
    }

    int bit_index = 7 - (index % 8);
    allocated_map[index / 8] ^= 1 << bit_index;
}

void Allocator::set_allocation_map_bit_at(int index, bool to_alloc)
{
    if (index == 0)
    {
        return;
    }

    int pair_index = index / 2 - (index % 2 == 0);
    int bit_index = 7 - (pair_index % 8);
    split_map[pair_index / 8] ^= 1 << bit_index;
}

void alloc_init_lists(int level)
{
    // just take the leafs you jerk
}

void Allocator::profile()
{
    for (size_t i = 0; i < LEVELS_COUNT; i++)
    {
        std::cout << "in for loop" << std::endl;

        LevelListNode *tmp = lists[i];
        std::cout << heap_beg << std::endl;
        std::cout << tmp << std::endl;
        while (tmp)
        {
            std::cout << ((size_t)heap_beg - (size_t)tmp) << " ";
            tmp = tmp->next;
        }
        std::cout << std::endl;
    }
}
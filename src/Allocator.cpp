#include <cstdlib>
#include <exception>
#include <cmath>
#include <iostream>
#include "../include/Allocator.h"

using std::cout;
using std::endl;

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
size_t Allocator::LEVELS_COUNT = log2(HEAP_SIZE / LEAF_SIZE) + 1;

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

    for (int lvl = LEVELS_COUNT - 1; lvl >= 0; lvl--)
    {
        size_t level_size = size_of_level(lvl);
        char *index_ptr = heap_beg;
        while (index_ptr < heap_beg + metadata_size)
        {
            int index = block_index(index_ptr, lvl);
            set_allocation_map_bit_at(index);
            set_split_map_bit_at(index, 1);
            index_ptr += level_size;
        }

        if (lvl != 0 && block_index_on_level(index_ptr, lvl) % 2 == 1)
        {
            lists[lvl] = (LevelListPointer)index_ptr;
            lists[lvl]->next = nullptr;
            lists[lvl]->prev = nullptr;
        }
    }
}

void *Allocator::allocate(size_t size)
{
    size_t return_block_size = compute_pow_2(size);
    int block_level = block_size_to_level(return_block_size);
    // void *block = get_block(block_level);
    // cout << "allocate from " << (char *)block - heap_beg
    //     << " to " << (char *)block - heap_beg + return_block_size << endl;
    // return block;
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
    block->prev = nullptr;

    set_allocation_map_bit_at(block_index((char *)block, level));
    return block;
}

void Allocator::split_blocks(int level)
{
    if (level == 0)
    {
        throw std::bad_alloc();
    }
    if (level > 0 && lists[level] == nullptr)
    {
        split_blocks(level - 1);
    }

    LevelListNode *current_level_block = lists[level];
    lists[level] = current_level_block->next;

    int current_level_block_index = block_index((char *)current_level_block, level);
    set_split_map_bit_at(current_level_block_index, 1);
    set_allocation_map_bit_at(current_level_block_index);

    lists[level + 1] = current_level_block;
    current_level_block->next = (LevelListNode *)((char *)current_level_block + size_of_level(level + 1));
    current_level_block->prev = nullptr;

    current_level_block->next->prev = current_level_block;
    current_level_block->next->next = nullptr;
}

void Allocator::free(void *ptr)
{
    int block_level = block_level_from_pointer((char *)ptr);
    int index = block_index((char *)ptr, block_level);

    // cout << "free block form " << (char *)ptr - heap_beg
    //     << " to " << (char *)ptr - heap_beg + size_of_level(block_level) << endl;
    set_allocation_map_bit_at(index);
    LevelListNode *block = (LevelListNode *)ptr;
    block->next = lists[block_level];
    lists[block_level] = block;
    if (!allocation_check(index))
    {
        int level_index = block_index_on_level((char *)ptr, block_level);
        merge_blocks(level_index, block_level);
    }
}

void Allocator::merge_blocks(int index, int level)
{
    if (level == 1) // perhaps could be 0 but will leave it as it is for now
    {
        return;
    }

    int buddy_index = index % 2 ? index - 1 : index + 1;

    size_t level_size = size_of_level(level);
    LevelListNode *buddy1 = (LevelListNode *)(heap_beg + level_size * index);
    LevelListNode *buddy2 = (LevelListNode *)(heap_beg + level_size * buddy_index);

    remove_node(lists[level], buddy1);
    remove_node(lists[level], buddy2);

    LevelListNode *merged_block = buddy1 < buddy2 ? buddy1 : buddy2;

    merged_block->next = lists[level - 1];
    lists[level - 1] = merged_block;
    int merged_block_index = block_index((char *)merged_block, level - 1);
    set_split_map_bit_at(merged_block_index, 0);
    set_allocation_map_bit_at(merged_block_index);

    if (!allocation_check(merged_block_index))
    {
        int level_index = block_index_on_level((char *)merged_block, level - 1);
        merge_blocks(level_index, level - 1);
    }
}

int Allocator::block_index_on_level(char *ptr, int level)
{
    return (ptr - heap_beg) / size_of_level(level);
}

int Allocator::block_index(char *ptr, int level)
{
    return block_index_on_level(ptr, level) + (1 << level) - 1;
}

int Allocator::block_size_to_level(size_t size)
{
    return LEVELS_COUNT - floor(log2(size / LEAF_SIZE)) - 1;
}

int Allocator::block_level_from_pointer(char *ptr)
{
    int level = LEVELS_COUNT - 1;
    while (level > 0)
    {
        if (block_has_been_split((char *)ptr, level - 1))
        {
            return level;
        }
        level--;
    }
    return 0;
}

void Allocator::set_split_map_bit_at(int index, bool bit)
{
    if (index >= (1 << LEVELS_COUNT) - 1)
    {
        return;
    }

    int bit_index = 7 - (index % 8);
    if (bit)
    {
        split_map[index / 8] |= 1 << bit_index;
    }
    else
    {
        split_map[index / 8] &= ~(1 << bit_index);
    }
}

void Allocator::set_allocation_map_bit_at(int index)
{
    if (index == 0)
    {
        return;
    }

    int pair_index = index / 2 - (index % 2 == 0);
    int bit_index = 7 - (pair_index % 8);
    // if (pair_index == 1)
    // {
    //     cout << "now for some reason" << endl;
    // }
    // cout << (int)allocated_map[pair_index / 8] << " prev value " << allocation_check(index) << " ";
    allocated_map[pair_index / 8] ^= 1 << bit_index;
    // cout << "pait index for alloc set " << pair_index << " "
    // << " bit index " << bit_index << " " << allocation_check(index) << endl;
}

void Allocator::profile()
{
    for (size_t i = 0; i < LEVELS_COUNT; i++)
    {
        if (lists[i] == nullptr)
        {
            std::cout << "pointer is null" << std::endl;
            continue;
        }
        std::cout << "pointer at lvl" << i << " " << (char *)lists[i] - heap_beg << std::endl;
    }

    // size_t blocks_to_split = 1 << (LEVELS_COUNT - 1);
    // size_t blocks_all = 1 << (LEVELS_COUNT);
    // for (size_t i = 0; i < blocks_to_split - 1; i++)
    // {
    //    cout << "split bit for " << i << " " << split_check(i) << endl;
    // }

    // for (size_t i = 0; i < blocks_all - 1; i++)
    // {
    //    cout << "allocated bit for " << i << " " << allocation_check(i) << endl;
    // }
}

bool Allocator::block_has_been_split(char *ptr, int level)
{
    int index = block_index(ptr, level);
    int bit_index = 7 - (index % 8);
    return (split_map[index / 8] >> bit_index) & 1;
}

bool Allocator::allocation_check(int index)
{
    if (index == 0)
    {
        return true;
    }
    int pair_index = index / 2 - (index % 2 == 0);
    int bit_index = 7 - (pair_index % 8);
    return (allocated_map[pair_index / 8] >> bit_index) & 1;
}

bool Allocator::split_check(int index)
{
    if (index >= (1 << LEVELS_COUNT) - 1)
    {
        return false;
    }

    int bit_index = 7 - (index % 8);
    return (split_map[index / 8] >> bit_index) & 1;
}

void Allocator::remove_node(LevelListPointer &list, LevelListNode *node)
{
    if (node->prev)
    {
        node->prev->next = node->next;
    }
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    if (node == list)
    {
        list = node->next;
    }
}
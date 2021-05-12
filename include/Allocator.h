#include <cstddef>
#include <cmath>

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#define MAX_LEVELS 32

class Allocator
{
private:
    const static size_t HEAP_SIZE = (1<<9);
    const static size_t LEAF_SIZE = (1<<3);
    // const static size_t INITIAL_SIZE = (1<<20);
    // const static size_t MIN_BLOCK_SIZE = (1<<4);
    static size_t LEVELS_COUNT;
    
    struct LevelListNode
    {
        LevelListNode * next;
    };

    enum Action
    {
        Alloc, Free 
    };

    using LevelListPointer = LevelListNode *;

    static Allocator * instance;
    char * heap_beg;
    size_t heap_size;
    LevelListPointer * lists;
    size_t lists_size;
    char * allocated_map;
    size_t allocated_map_size;
    char * split_map;
    size_t split_map_size;

    Allocator();
    void init_metadata();
    size_t size_of_level(int n);
    int block_index_on_level(char * ptr, int level);
    int block_size_to_level(size_t size);
    int index_in_level(char * ptr, int level);
    void * get_block(int level);
    void set_allocation_map_bit_at(int index, bool to_alloc);
    void set_split_map_bit_at(int index, bool to_alloc);
    void split_blocks(int level);

    void alloc_init_lists(int level);
public:
    static Allocator * get_instance();

    void * allocate(size_t size);
    void free();
    ~Allocator();

    void profile();
};

#endif
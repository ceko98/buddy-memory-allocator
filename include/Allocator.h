#include <cstddef>
#include <cmath>
#include <cstdint>
#include <mutex>

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#define MAX_LEVELS 32

class Allocator
{
private:
    static std::mutex init_mutex;
    static std::mutex alloc_free_mutex;

    const static size_t HEAP_SIZE = (1<<10);
    const static size_t LEAF_SIZE = (1<<5);
    // const static size_t INITIAL_SIZE = (1<<20);
    // const static size_t MIN_BLOCK_SIZE = (1<<4);
    static size_t LEVELS_COUNT;
    
    struct LevelListNode
    {
        LevelListNode * next;
        LevelListNode * prev;
    };
    typedef LevelListNode * LevelListPointer;

    static Allocator * instance;
    uint8_t * heap_beg;
    size_t heap_size;
    LevelListPointer *lists;
    uint8_t * allocation_map;
    uint8_t * split_map;

    Allocator();
    void init_metadata();

    size_t size_of_level(int n);
    int block_index_on_level(uint8_t * ptr, int level);
    int block_index(uint8_t * ptr, int level);
    int block_size_to_level(size_t size);

    void set_allocation_map_bit_at(int index);
    void set_split_map_bit_at(int index, bool bit);
    
    bool block_has_been_split(uint8_t *ptr, int level);
    int block_level_from_pointer(uint8_t *ptr);
    bool allocation_check(int index);
    bool split_check(int index);

    void * get_block(int level);
    void split_blocks(int level);
    void merge_blocks(int pair_index, int block_level);

    void remove_node(LevelListPointer &list, LevelListNode*  node);

    void * operator new(size_t sz);
    void operator delete(void * ptr);
public:
    static Allocator * get_instance();

    void * allocate(size_t size);
    void free(void * ptr);
    ~Allocator();

    void profile();
};

#endif
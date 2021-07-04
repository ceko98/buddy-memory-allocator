#include <cstddef>
#include <cmath>
#include <cstdint>
#include <mutex>

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#define MAX_LEVELS 32
#define MIN_LEAF_SIZE 16

class Allocator
{
private:
    static std::mutex init_mutex;
    static std::mutex alloc_free_mutex;

    /*
    Change these values to adjust:
    - the total allocated space
    - the size of the minimal allocated block 
    */
    const static size_t HEAP_SIZE = (1<<10);
    const static size_t LEAF_SIZE = (1<<5);
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
    bool debug;

    Allocator(bool debug);
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
    static Allocator * get_instance(bool debug = false);

    void * allocate(size_t size);
    void free(void * ptr);
    ~Allocator();
};

#endif
#include <cstddef>
#include <cmath>

#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

class Allocator
{
private:
    const static size_t INITIAL_SIZE = (1<<20);
    const static size_t MIN_BLOCK_SIZE = (1<<4);
    static size_t LEVELS_COUNT;
    
    struct LevelListNode
    {
        void *prev = nullptr;
        void *next = nullptr;
    };

    static Allocator *instance;
    void *heap_beg;
    size_t heap_size;
    LevelListNode **lists;
    size_t lists_size;
    void *bitmap;
    size_t bitmap_size;

    Allocator();
    void init_lists();
    
public:
    static Allocator *get_instance();

    void *allocate(size_t size);
    void free();
    ~Allocator();
};

#endif
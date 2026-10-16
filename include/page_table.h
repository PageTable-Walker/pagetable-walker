#pragma once
#include "frame_allocator.h"
#include <cstdint>


static constexpr uint64_t PT_ENTRIES = 512;   
static constexpr int      PT_LEVELS  = 4;     

static constexpr uint8_t PERM_READ     = 1 << 0;
static constexpr uint8_t PERM_WRITE    = 1 << 1;
static constexpr uint8_t PERM_EXEC     = 1 << 2;
static constexpr uint8_t PERM_ACCESSED = 1 << 3;
static constexpr uint8_t PERM_DIRTY    = 1 << 4;

struct PageTableNode {
    PageTableNode* child[PT_ENTRIES];
    uint64_t       frame[PT_ENTRIES];
    uint8_t        flags[PT_ENTRIES];
    bool           present[PT_ENTRIES];

    PageTableNode();
};

struct WalkResult {
    bool           present;
    uint64_t       frame_number;
    uint8_t        flags;
    PageTableNode* leaf_node;
    uint64_t       leaf_index;
};

class PageTable {
public:
    explicit PageTable(FrameAllocator& allocator);
    ~PageTable();

    // Maps the page containing `vaddr` to a freshly allocated frame.
    // Creates any missing intermediate tree nodes along the way.
    // Throws std::runtime_error if already mapped or out of frames.
    void map(uint64_t vaddr, uint8_t perm);

    // Unmaps the page at `vaddr` if present, and frees its frame.
    // Does nothing if not mapped.
    void unmap(uint64_t vaddr);

    // Walks the tree without creating any missing nodes.
    WalkResult walk(uint64_t vaddr);

private:
    PageTableNode*   root;
    FrameAllocator&  allocator_;

    static uint64_t index_at_level(uint64_t vaddr, int level);
    PageTableNode*   get_or_create_child(PageTableNode* node, uint64_t idx);
    void             free_subtree(PageTableNode* node, int level);

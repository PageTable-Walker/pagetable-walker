#include "page_table.h"
#include <stdexcept>

PageTableNode::PageTableNode() {
    for (uint64_t i = 0; i < PT_ENTRIES; ++i) {
        child[i]   = nullptr;
        frame[i]   = 0;
        flags[i]   = 0;
        present[i] = false;
    }
}

PageTable::PageTable(FrameAllocator& allocator)
    : allocator_(allocator)
{
    root = new PageTableNode();
}

PageTable::~PageTable() {
    free_subtree(root, 0);
}

void PageTable::free_subtree(PageTableNode* node, int level) {
    if (node == nullptr) return;

    if (level < PT_LEVELS - 1) {
        for (uint64_t i = 0; i < PT_ENTRIES; ++i) {
            free_subtree(node->child[i], level + 1);
        }
    }
    delete node;
}

uint64_t PageTable::index_at_level(uint64_t vaddr, int level) {
    int shift = 12 + (PT_LEVELS - 1 - level) * 9;
    return (vaddr >> shift) & (PT_ENTRIES - 1);
}

PageTableNode* PageTable::get_or_create_child(PageTableNode* node, uint64_t idx) {
    if (node->child[idx] == nullptr) {
        node->child[idx] = new PageTableNode();
    }
    return node->child[idx];
}

void PageTable::map(uint64_t vaddr, uint8_t perm) {
    PageTableNode* node = root;

    for (int level = 0; level < PT_LEVELS - 1; ++level) {
        uint64_t idx = index_at_level(vaddr, level);
        node = get_or_create_child(node, idx);
    }

    uint64_t leaf_idx = index_at_level(vaddr, PT_LEVELS - 1);
    if (node->present[leaf_idx]) {
        throw std::runtime_error("PageTable::map: address already mapped");
    }

    uint64_t frame_number = allocator_.allocate();   // throws if out of frames

    node->frame[leaf_idx]   = frame_number;
    node->flags[leaf_idx]   = perm;
    node->present[leaf_idx] = true;
}

void PageTable::unmap(uint64_t vaddr) {
    PageTableNode* node = root;

    for (int level = 0; level < PT_LEVELS - 1; ++level) {
        uint64_t idx = index_at_level(vaddr, level);
        if (node->child[idx] == nullptr) {
            return;   
        }
        node = node->child[idx];
    }

    uint64_t leaf_idx = index_at_level(vaddr, PT_LEVELS - 1);
    if (!node->present[leaf_idx]) {
        return;   // already unmapped
    }

    allocator_.free(node->frame[leaf_idx]);

    node->present[leaf_idx] = false;
    node->frame[leaf_idx]   = 0;
    node->flags[leaf_idx]   = 0;

    // NOTE: if/when a TLB is added, invalidate (asid, vaddr >> 12) here.
    // A stale TLB entry after this point would serve the freed frame's
    // old translation to the next lookup.
}

WalkResult PageTable::walk(uint64_t vaddr) {
    PageTableNode* node = root;

    for (int level = 0; level < PT_LEVELS - 1; ++level) {
        uint64_t idx = index_at_level(vaddr, level);
        if (node->child[idx] == nullptr) {
            return WalkResult{ false, 0, 0, nullptr, 0 };
        }
        node = node->child[idx];
    }

    uint64_t leaf_idx = index_at_level(vaddr, PT_LEVELS - 1);
    if (!node->present[leaf_idx]) {
        return WalkResult{ false, 0, 0, nullptr, 0 };
    }

    return WalkResult{
        true,
        node->frame[leaf_idx],
        node->flags[leaf_idx],
        node,
        leaf_idx
    };

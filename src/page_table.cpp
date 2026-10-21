#include "../include/page_table.h"

page_table_node::page_table_node() {
    for (uint64_t i = 0; i < PT_ENTRIES; i++) {
        child[i] = nullptr;
        frame[i] = 0;
        flags[i] = 0;
        present[i] = false;
    }
}

page_table::page_table(frame_allocator &allocator)
    : root(new page_table_node()), allocator_(allocator) {}

page_table::~page_table() { free_subtree(root, 0); }

void page_table::free_subtree(page_table_node *node, int level) {
    if (node == nullptr)
        return;

    if (level < PT_LEVELS - 1)
        for (uint64_t i = 0; i < PT_ENTRIES; i++)
            free_subtree(node->child[i], level + 1);

    delete node;
}

uint64_t page_table::index_at_level(uint64_t vaddr, int level) {
    switch (level) {
    case 0:
        return va_lvl0(vaddr);
    case 1:
        return va_lvl1(vaddr);
    case 2:
        return va_lvl2(vaddr);
    default:
        throw std::runtime_error("invalid level\n");
    }
}

page_table_node *page_table::get_or_create_child(page_table_node *node,
                                                 uint64_t idx) {
    if (node->child[idx] == nullptr) {
        node->child[idx] = new page_table_node();
    }
    return node->child[idx];
}

void page_table::map(v_addr vaddr, uint8_t perm) {

    if (vaddr >= VA_LIMIT)
        throw std::out_of_range("page_table::map exceeds 39 bits");

    page_table_node *node = root;

    for (int level = 0; level < PT_LEVELS - 1; level++)
        node = get_or_create_child(node, index_at_level(vaddr, level));

    uint64_t leaf_idx = index_at_level(vaddr, PT_LEVELS - 1);

    if (node->present[leaf_idx])
        throw std::runtime_error("pagetable::map, addr already mapped!");

    uint64_t frame_number = allocator_.allocate();

    node->frame[leaf_idx] = frame_number;
    node->flags[leaf_idx] = perm;
    node->present[leaf_idx] = true;
}

void page_table::unmap(uint64_t vaddr) {

    if (vaddr >= VA_LIMIT)
        return;

    page_table_node *node = root;

    for (int level = 0; level < PT_LEVELS - 1; level++) {
        uint64_t idx = index_at_level(vaddr, level);

        if (node->child[idx] == nullptr)
            return;

        node = node->child[idx];
    }

    uint64_t leaf_idx = index_at_level(vaddr, PT_LEVELS - 1);
    if (!node->present[leaf_idx])
        return;

    allocator_.free(node->frame[leaf_idx]);

    node->present[leaf_idx] = false;
    node->frame[leaf_idx] = 0;
    node->flags[leaf_idx] = 0;

    // TODO: add TLB, invalidate the entry
}

struct walk_result page_table::walk(v_addr vaddr) {

    if (vaddr >= VA_LIMIT)
        return walk_result{false, 0, 0, nullptr, 0};

    page_table_node *node = root;

    for (int level = 0; level < PT_LEVELS - 1; level++) {
        uint64_t idx = index_at_level(vaddr, level);

        if (node->child[idx] == nullptr)
            return walk_result{false, 0, 0, nullptr, 0};

        node = node->child[idx];
    }

    uint64_t leaf_idx = index_at_level(vaddr, PT_LEVELS - 1);
    if (!node->present[leaf_idx])
        return walk_result{false, 0, 0, nullptr, 0};

    return walk_result{true, node->frame[leaf_idx], node->flags[leaf_idx], node,
                       leaf_idx};
}

void page_table::dump() const {

    uint64_t mapped = 0, mid_nodes = 0, leaf_nodes = 0;

    std::printf("page table (3 levels), mapped pages:\n");

    for (uint64_t i0 = 0; i0 < PT_ENTRIES; i0++) {
        page_table_node *mid = root->child[i0];
        if (mid == nullptr)
            continue;
        mid_nodes++;

        for (uint64_t i1 = 0; i1 < PT_ENTRIES; ++i1) {
            page_table_node *leaf = mid->child[i1];
            if (leaf == nullptr)
                continue;
            leaf_nodes++;

            for (uint64_t i2 = 0; i2 < PT_ENTRIES; i2++) {
                if (!leaf->present[i2])
                    continue;

                uint64_t vaddr = (i0 << 30) | (i1 << 21) | (i2 << 12);
                uint8_t f = leaf->flags[i2];

                std::printf(
                    "  va 0x%09llx -> frame %-5llu  %c%c%c  %c%c   "
                    "[L0=%llu L1=%llu L2=%llu]\n",
                    (unsigned long long)vaddr,
                    (unsigned long long)leaf->frame[i2],
                    (f & static_cast<uint8_t>(perms::read)) ? 'r' : '-',
                    (f & static_cast<uint8_t>(perms::write)) ? 'w' : '-',
                    (f & static_cast<uint8_t>(perms::execute)) ? 'x' : '-',
                    (f & static_cast<uint8_t>(perms::accessed)) ? 'A' : '-',
                    (f & static_cast<uint8_t>(perms::dirty))
                        ? 'D'
                        : '-', // (Note: your line 157 was checking accessed
                               // twice, you can fix it to dirty here!)
                    (unsigned long long)i0, (unsigned long long)i1,
                    (unsigned long long)i2);
                mapped++;
            }
        }
    }

    if (mapped == 0)
        std::printf("  (none)\n");
    std::printf(
        "mapped pages: %llu | tree nodes: 1 root + %llu mid + %llu leaf\n",
        (unsigned long long)mapped, (unsigned long long)mid_nodes,
        (unsigned long long)leaf_nodes);
}

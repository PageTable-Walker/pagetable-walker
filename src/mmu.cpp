#include "../include/mmu.h"

mmu_::mmu_(page_table &pt, physical_mem &mem) : pt_(pt), mem_(mem) {}

uint64_t mmu_::page_offset(uint64_t vaddr) { return vaddr & (FRAME_SZ - 1); }

uint64_t mmu_::translate(uint64_t vaddr, bool is_write) {

    struct walk_result walk = pt_.walk(vaddr);

    if (!walk.present)
        throw page_fault(fault_kind::NOT_PRESENT, "page not present");

    if (is_write && !(walk.flags & static_cast<uint8_t>(perms::write)))
        throw page_fault(fault_kind::PROTECTION, "write to r-only page");

    if (!is_write && !(walk.flags & static_cast<uint8_t>(perms::read)))
        throw page_fault(fault_kind::PROTECTION, "read to non-readable page");

    walk.leaf_node->flags[walk.leaf_index] |=
        static_cast<uint8_t>(perms::accessed);
    if (is_write)
        walk.leaf_node->flags[walk.leaf_index] |=
            static_cast<uint8_t>(perms::dirty);

    uint64_t paddr = (walk.frame_number * FRAME_SZ) + page_offset(vaddr);
    return paddr;
}

uint8_t mmu_::read_byte(uint64_t vaddr) {

    uint64_t paddr = translate(vaddr, false);
    return mem_.read_byte(paddr);
}

void mmu_::write_byte(uint64_t vaddr, uint8_t value) {

    uint64_t paddr = translate(vaddr, true);
    mem_.write_byte(paddr, value);
}

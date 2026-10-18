#include "mmu.h"

Mmu::Mmu(PageTable& pt, PhysicalMemory& mem)
    : pt_(pt), mem_(mem)
{
}

uint64_t Mmu::page_offset(uint64_t vaddr) {
    return vaddr & (FrameAllocator::FRAME_SZ - 1);
}

uint64_t Mmu::translate(uint64_t vaddr, bool is_write) {
    WalkResult w = pt_.walk(vaddr);

    if (!w.present) {
        throw PageFault(FaultKind::NOT_PRESENT, "page not present");
    }

    if (is_write && !(w.flags & PERM_WRITE)) {
        throw PageFault(FaultKind::PROTECTION, "write to read-only page");
    }

    w.leaf_node->flags[w.leaf_index] |= PERM_ACCESSED;
    if (is_write) {
        w.leaf_node->flags[w.leaf_index] |= PERM_DIRTY;
    }

    uint64_t paddr = (w.frame_number * FrameAllocator::FRAME_SZ) + page_offset(vaddr);
    return paddr;
}

uint8_t Mmu::read_byte(uint64_t vaddr) {
    uint64_t paddr = translate(vaddr, /*is_write=*/false);
    return mem_.read_byte(paddr);
}

void Mmu::write_byte(uint64_t vaddr, uint8_t value) {
    uint64_t paddr = translate(vaddr, /*is_write=*/true);
    mem_.write_byte(paddr, value);
}

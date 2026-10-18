#pragma once
#include "page_table.h"
#include "physical_mem.h"
#include "frame_allocator.h"
#include <stdexcept>
#include <cstdint>

enum class FaultKind { NOT_PRESENT, PROTECTION };

class PageFault : public std::runtime_error {
public:
    FaultKind kind;
    PageFault(FaultKind k, const char* msg) : std::runtime_error(msg), kind(k) {}
};

class Mmu {
public:
    Mmu(PageTable& pt, PhysicalMemory& mem);
    uint64_t translate(uint64_t vaddr, bool is_write);

    uint8_t  read_byte(uint64_t vaddr);
    void     write_byte(uint64_t vaddr, uint8_t value);

private:
    PageTable&      pt_;
    PhysicalMemory& mem_;

    static uint64_t page_offset(uint64_t vaddr);
};

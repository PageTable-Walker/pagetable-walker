#include "../include/physical_mem.h"
#include <cstring>

uint8_t physical_mem::mem[PHYS_MEM_SZ];

physical_mem::physical_mem() { std::memset(mem, 0, PHYS_MEM_SZ); }

void physical_mem::check_bounds(uint64_t paddr, uint64_t access_sz) const {

    if (paddr + access_sz > PHYS_MEM_SZ)
        throw std::out_of_range("physical addr out of bounds");
}

uint8_t physical_mem::read_byte(p_addr addr) const {

    check_bounds(addr, 1);
    return mem[addr];
}
void physical_mem::write_byte(p_addr addr, uint8_t value) {

    check_bounds(addr, 1);
    mem[addr] = value;
}

// simulating ordinary 64 bit val access
uint64_t physical_mem::read_u64(uint64_t addr) const {

    check_bounds(addr, sizeof(uint64_t));
    uint64_t value;
    std::memcpy(&value, &mem[addr], sizeof(uint64_t));
    return value;
}

void physical_mem::write_u64(uint64_t addr, uint64_t value) {

    check_bounds(addr, sizeof(uint64_t));
    std::memcpy(&mem[addr], &value, sizeof(uint64_t));
}

void physical_mem::zero_range(uint64_t addr, uint64_t len) {

    check_bounds(addr, len);
    std::memset(&mem[addr], 0, len);
}

#include "pte.h"
#include <cstdint>

#define PHYS_MEM_SZ 128 * 1024 * 1024
#define FRAME_SZ 4096

class physical_mem {
  private:
    static uint8_t bytes[PHYS_MEM_SZ];

  public:
    const uint64_t phys_mem_sz() { return PHYS_MEM_SZ; }
    const uint64_t frame_count() { return PHYS_MEM_SZ / FRAME_SZ; }

    const uint8_t read_byte(p_addr addr);
    uint8_t write_byte(p_addr addr);

    void zero_frame(uint64_t frame_number);
};

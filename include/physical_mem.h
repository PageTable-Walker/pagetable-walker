#include <cstdint>
#include <stdexcept>

#define PHYS_MEM_SZ 16ULL * 1024 * 1024 // simulating 16 MiB
#define FRAME_SZ 4096

typedef uint64_t p_addr;

class physical_mem {
  private:
    static uint8_t mem[PHYS_MEM_SZ];
    void check_bounds(uint64_t paddr, uint64_t access_sz) const;

  public:
    physical_mem();

    static uint64_t phys_mem_sz() { return PHYS_MEM_SZ; }
    static uint64_t frame_count() { return PHYS_MEM_SZ / FRAME_SZ; }

    uint8_t read_byte(p_addr addr) const;
    void write_byte(p_addr addr, uint8_t value);

    // simulating ordinary 64 bit val access
    uint64_t read_u64(uint64_t addr) const;
    void write_u64(uint64_t addr, uint64_t value);

    void zero_range(uint64_t addr, uint64_t len);
};

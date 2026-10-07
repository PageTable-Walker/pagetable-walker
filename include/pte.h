#include <cstdint>

#define PAGE_SIZE 4096
#define ENTRIES_PER_TABLE 512

typedef uint64_t v_addr;
typedef uint64_t p_addr;
typedef uint64_t pte;

#define TABLE_BYTES (ENTRIES_PER_TABLE * sizeof(pte))

//---- decoding virtual address ----//
inline uint64_t va_pml4_idx(v_addr va) { return (va >> 39) & 0x1FFull; }
inline uint64_t va_pdpt_idx(v_addr va) { return (va >> 30) & 0x1FFull; }
inline uint64_t va_pt_idx(v_addr va) { return (va >> 21) & 0x1FFull; }
inline uint64_t va_offset(v_addr va) { return (va >> 12) & 0x1FFull; }

//---- defining pte permissions ----//

enum pte_flag {
    Present = 1 << 0,
    Writable = 1 << 1,
    User = 1 << 2,
    Accessed = 1 << 5,
    Dirty = 1 << 6,
    NoExecute = 1ull << 63,
};

enum perms {
    None = 0,
    Read = 1 << 0,
    Write = 1 << 1,
    Execute = 1 << 22,
};

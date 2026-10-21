#include "frame_allocator.h"

#define PAGE_SIZE 4096
#define PT_ENTRIES 512
#define PT_LEVELS 3 // choosing 3 levels for the sake of simplicity
#define VA_BITS 39
#define VA_LIMIT (1ull << VA_BITS)

typedef uint64_t v_addr;
typedef uint64_t pte;

#define TABLE_BYTES (PT_ENTRIES * sizeof(pte))

//---- decoding virtual address (3-level, 39-bit space) ----//

inline uint64_t va_lvl0(v_addr va) { return (va >> 30) & 0x1FFull; }
inline uint64_t va_lvl1(v_addr va) { return (va >> 21) & 0x1FFull; }
inline uint64_t va_lvl2(v_addr va) { return (va >> 12) & 0x1FFull; }
inline uint64_t va_offset(v_addr va) { return va & 0xFFFull; } // 12-bit offset

//---- defining pte permissions ----//

enum class perms {
    none = 0,
    read = 1 << 0,
    write = 1 << 1,
    execute = 1 << 3,
    accessed = 1 << 5,
    dirty = 1 << 6,
    present = 1 << 7,
};

struct page_table_node {
    page_table_node *child[PT_ENTRIES];
    uint64_t frame[PT_ENTRIES];
    uint8_t flags[PT_ENTRIES];
    bool present[PT_ENTRIES];

    page_table_node();
};

struct walk_result {
    bool present;
    uint64_t frame_number;
    uint8_t flags;
    page_table_node *leaf_node; // to flip accessed/dirty bit
    uint64_t leaf_index;
};

class page_table {

  public:
    page_table(frame_allocator &allocator);
    ~page_table();

    // maps page containing vaddr to fresh frame.
    // creates missing tree nodes and throws err if already mapped
    void map(uint64_t vaddr, uint8_t perm);

    void unmap(uint64_t vaddr);

    struct walk_result walk(uint64_t vaddr);

    void dump() const;

  private:
    page_table_node *root;
    frame_allocator &allocator_;

    static uint64_t index_at_level(uint64_t vaddr, int level);
    page_table_node *get_or_create_child(page_table_node *node, uint64_t idx);
    void free_subtree(page_table_node *node, int level);

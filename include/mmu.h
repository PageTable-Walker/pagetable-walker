#include "page_table.h"

enum class fault_kind { NOT_PRESENT, PROTECTION };

class page_fault : public std::runtime_error {
  public:
    fault_kind kind;
    page_fault(fault_kind k, const char *msg)
        : std::runtime_error(msg), kind(k) {}
};

class mmu_ {
  public:
    mmu_(page_table &pt, physical_mem &mem);

    // returns physical_addr
    uint64_t translate(uint64_t vaddr, bool is_write);

    // wrappers for translating then touching physical mem
    uint8_t read_byte(uint64_t vaddr);
    void write_byte(uint64_t vaddr, uint8_t value);

  private:
    page_table &pt_;
    physical_mem &mem_;

    static uint64_t page_offset(uint64_t vaddr);
};

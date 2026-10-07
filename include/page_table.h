
#include "asid_table.h"
#include "frame_allocator.h"
#include "physical_mem.h"
#include "pte.h"

class page_table {
  private:
    physical_mem &mem;
    frame_allocator &allocator;
    asid_table &asid;

  public:
    page_table(physical_mem &mem, frame_allocator &allocator,
               asid_table &asids);

    void map(uint16_t asid, v_addr va, perm perm);

    void unmap(uint16_t asid, v_addr va);
}

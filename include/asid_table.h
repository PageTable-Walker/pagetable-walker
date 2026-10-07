#include "physical_mem.h"

#define MAX_ASID 256

class asid_table {
  public:
    asid_table();

    // 0 means "no root allocated yet"
    uint64_t root_frame(uint16_t asid);
    void set_root(uint16_t asid, uint64_t frame_number);

  private:
    uint64_t roots[MAX_ASID];
};

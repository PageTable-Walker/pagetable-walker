#include "physical_mem.h"
#include <cstdint>

#define TOTAL_FRAMES (PHYS_MEM_SZ / FRAME_SZ)
#define BITMAP_COUNT ((TOTAL_FRAMES + 63) / 64)

// bitmap's bit is set to 0 -> frame is used
// bitmap's bit is set to 1 -> frame is free
class frame_allocator {
  private:
    // 1 single uint64_t value will track 64 frames.
    uint64_t bitmap[BITMAP_COUNT];

    uint64_t free_frames_;

    void set_bit(uint64_t n);
    void clear_bit(uint64_t n);

  public:
    const bool is_used(uint64_t frame_number);
    void mark_used(uint64_t frame_number);

    frame_allocator();

    // Allocate one free frame and returns frame number, otherwise reports error
    uint64_t allocate();

    // Free a previously allocated frame.
    void free(uint64_t frame_number);

    // Query state.
    bool is_free(uint64_t frame_number);

    // Stats.
    uint64_t free_count() const { return free_frames_; }
    uint64_t total_count() const { return TOTAL_FRAMES; }
};

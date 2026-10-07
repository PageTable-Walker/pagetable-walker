#include "../include/frame_allocator.h"
#include <cstring> //for std::memset
#include <stdexcept> // for cpp errors that need to be repalced by custom error handling

frame_allocator::frame_allocator() {

    free_frames_ = TOTAL_FRAMES;

    std::memset(bitmap, 0xFF, BITMAP_COUNT); // mark every frame free
}

void frame_allocator::clear_bit(uint64_t frame_number) {}

void frame_allocator::set_bit(uint64_t frame_number) {}

bool frame_allocator::is_free(uint64_t frame_number) { return false; }

uint64_t frame_allocator::allocate() {

    // TODO: replace throw std::runtime_error with custom error and tracing
    if (free_frames_ == 0)
        throw std::runtime_error("frame_allocator : no free frames\n");

    return 0x0;
}

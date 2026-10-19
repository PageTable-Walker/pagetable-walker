#include "../include/frame_allocator.h"
#include <cstring> //for std::memset
#include <stdexcept> // for cpp errors that need to be repalced by custom error handling

frame_allocator::frame_allocator() {

    free_frames_ = TOTAL_FRAMES;

    std::memset(bitmap, 0xFF, BITMAP_COUNT); // mark every frame free
}

void frame_allocator::clear_bit(uint64_t frame_number) {
    bitmap[frame_number / 64] &= ~(1ULL << (frame_number % 64));
}

void frame_allocator::set_bit(uint64_t frame_number) {
    bitmap[frame_number / 64] |= (1ULL << (frame_number % 64));
}
bool frame_allocator::is_free(uint64_t frame_number) {
    return (bitmap[frame_number / 64] & (1ULL << (frame_number % 64))) != 0;
}
uint64_t frame_allocator::allocate() {
    if (free_frames_ == 0)
        throw std::runtime_error("No free frames");

    for (uint64_t i = 0; i < TOTAL_FRAMES; i++) {
        if (is_free(i)) {
            clear_bit(i);
            free_frames_--;
            return i;
        }
    }

    throw std::runtime_error("No free frames");
}
void frame_allocator::free(uint64_t frame_number) {
    if (frame_number >= TOTAL_FRAMES)
        throw std::runtime_error("Invalid frame number");

    if (!is_free(frame_number)) {
        set_bit(frame_number);
        free_frames_++;
    }
}

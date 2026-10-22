cmake_minimum_required(VERSION 4.4)
project(pagetable-walker C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# TODO (TLB phase): add src/tlb.cpp here later.
add_library(ptw_core STATIC
    src/physical_mem.cpp
    src/frame_allocator.cpp
    src/page_table.cpp
    src/mmu.cpp)
target_include_directories(ptw_core PUBLIC src)

add_executable(ptwalker src/cli.cpp)
target_link_libraries(ptwalker PRIVATE ptw_core)

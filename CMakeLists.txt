cmake_minimum_required(VERISON 4.4)
project(pagetable-walker C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall -Wextra)

# TODOO: add_library functionality. use for tests and main exe
add_executable(ptwalker
    src/cli.cpp
    src/physical_mem.cpp
    src/asid_table.cpp
    src/page_table.cpp
    src/tlb.cpp
    src/mmu.cpp)


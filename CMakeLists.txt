cmake_minimum_required(VERSION 2.8)
project(zalloc)

SET (CMAKE_C_COMPILER "clang")
SET (CMAKE_C_FLAGS "-Wall -ggdb3 -O3 -fheinous-gnu-extensions -fno-builtin -march=amdfam10 -mcx16 -mpopcnt -fno-builtin -ffast-math")

include_directories("./include/")

SET (ZM_SOURCES
    src/maps.c
    src/alloc.c
#    src/bitmap.c
)

ADD_LIBRARY(zalloc SHARED ${ZM_SOURCES})
TARGET_LINK_LIBRARIES(zalloc dl)


add_executable(mmap_test test/mmap_test.c)
TARGET_LINK_LIBRARIES(mmap_test zalloc)


#add_executable(alloc_test test/alloc_test.c)
#TARGET_LINK_LIBRARIES(alloc_test zalloc)

#add_executable(bmap_test test/bmap_test.c)
#TARGET_LINK_LIBRARIES(bmap_test zalloc)

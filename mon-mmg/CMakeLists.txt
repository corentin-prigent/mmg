cmake_minimum_required(VERSION 3.10)
project(TestMmg2d C)

set(mmg_DIR "/home/eelleb/Desktop/mmg/install/lib/cmake/mmg")

set(MMG_SOURCE_DIR "/home/eelleb/Desktop/mmg")

find_package(mmg REQUIRED)   # une seule fois

add_executable(mon_test src/main.c)

target_link_libraries(mon_test PRIVATE mmg2d m)

target_include_directories(mon_test PRIVATE
    ${MMG_SOURCE_DIR}/src/mmg2d
    ${MMG_SOURCE_DIR}/src/common
)

add_executable(mon_test2 src/main2.c)
target_link_libraries(mon_test2 PRIVATE mmg2d m)
target_include_directories(mon_test2 PRIVATE
    ${MMG_SOURCE_DIR}/src/mmg2d
    ${MMG_SOURCE_DIR}/src/common
)
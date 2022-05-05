cmake_minimum_required(VERSION 3.21)
project(yalibwav C)

set(CMAKE_C_STANDARD 99)

add_library(yalibwav SHARED yalibwav.c)
set_target_properties(yalibwav PROPERTIES PUBLIC_HEADER "yalibwav.h")

install(TARGETS yalibwav LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)

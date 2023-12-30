cmake_minimum_required(VERSION 3.21)

project(minecraft-hook LANGUAGES C CXX ASM)

add_subdirectory(external/jnihook)

add_library(mchook SHARED src/main.cpp)
target_link_libraries(mchook PRIVATE jnihook -static-libgcc -static-libstdc++)
set_target_properties(mchook PROPERTIES POSITION_INDEPENDENT_CODE True)

project(PakLib LANGUAGES C CXX)

add_library(PakLib STATIC
    PakInterface.cpp
)

target_include_directories(PakLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../compat
)
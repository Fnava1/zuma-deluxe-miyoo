cmake_minimum_required(VERSION 3.13)
project(ZumaDecomp C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    set(ARCH_FLAGS "-marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -fsigned-char -funwind-tables -rdynamic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH_FLAGS} -O3 -fno-strict-aliasing -ftree-vectorize -fomit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS} -O3 -fno-strict-aliasing -ftree-vectorize -fomit-frame-pointer")
    add_definitions(-D_GNU_SOURCE)
    add_definitions(-DSEXY_DISABLE_MODVAL)
endif()

if(MSVC)
    message(STATUS "MSVC Version: ${MSVC_VERSION}")
    if (MSVC_VERSION EQUAL 1300)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GF /EHsc /GS /Gy /GR /W3 /c /Wp64 /Z7 /TP")
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Og /Ob1 /Oi /Ot /Oy-")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    else()
        add_definitions(-DNOMINMAX)
        if(CMAKE_CXX_FLAGS MATCHES "sanitize=address")
            message(STATUS "ASAN detected, switching to non-debug runtime library.")
            set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
        endif()
    endif()
    add_definitions(-D_USE_MATH_DEFINES)
endif()

set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC ON CACHE BOOL "" FORCE)
set(LIBXMP_PIC ON CACHE BOOL "" FORCE)
add_subdirectory(3rdparty/libxmp-lite)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/source
    ${CMAKE_CURRENT_SOURCE_DIR}/source/compat
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SDL2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/SDL2/include/SDL2
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/libxmp-lite/include/libxmp-lite
)

add_subdirectory(source/ImageLib)
add_subdirectory(source/PakLib)
add_subdirectory(source/SexyAppFramework)
set(SEXYAPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source)
add_subdirectory(source/CircleShoot)
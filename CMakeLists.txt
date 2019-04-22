cmake_minimum_required(VERSION 3.9.1)

project(ccs DESCRIPTION "ccs configuration library")

set(BOOST_INCLUDEDIR include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# All the sanitizer flags in one place
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -fsanitize=address")
    set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -fsanitize=undefined")
else ()
    set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -static-libasan")
    set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -static-libubsan")
    set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -fsanitize=address")
    set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -fsanitize=signed-integer-overflow")
    set(SANITIZE_FLAGS "${SANITIZE_FLAGS} -fsanitize=bounds-strict -fsanitize=undefined")
endif ()

# Common flags shared across all builds
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall -Wextra -Werror -g")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZE_FLAGS}")

include(CTest)

# Enable libtooling support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# External libraries/dependencies
find_package(Boost 1.55.0 REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
add_subdirectory(external/googletest)

# au configuration
include_directories(api src)
link_directories(${PROJECT_BINARY_DIR}/lib)

add_subdirectory(src)
enable_testing()
add_subdirectory(test)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/dist)
option(LTO "Use link-time optimization" OFF)

if (LTO)
    message("Enabling link-time optimization")
    include(CheckIPOSupported)
    check_ipo_supported()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

# clang -Wextra only for our own code
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-undef -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-shadow-field-in-constructor -Wno-missing-prototypes -Wno-padded -Wno-float-equal -Wno-missing-noreturn -Wno-vla -Wno-vla-extension")
    # TODO either fix documentation or remove it, then remove this
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-documentation")
    # TODO decide if this is OK. It probably is
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-exit-time-destructors")
    # TODO should probably remove this and fix unnecessary things once API has settled.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unneeded-member-function -Wno-unused-member-function -Wno-unused-template")
    # TODO if we ever go non-header only, we can improve things a tiny bit by bringing some of the vtable definitions out of line
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-weak-vtables")
else ()
    # enable some more gcc warnings to match clang
    # TODO tidy up the code and bring these back...
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion -Wold-style-cast")
endif ()

add_library(libccs SHARED
    context.cpp
    dag/key.cpp
    dag/property.cpp
    dag/tally.cpp
    domain.cpp
    graphviz.cpp
    parser/ast.cpp
    parser/build_context.cpp
    parser/parser.cpp
    rule_builder.cpp
    search_state.cpp)
#set_target_properties(libccs PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(libccs PROPERTIES PUBLIC_HEADER ../api)
target_include_directories(libccs PRIVATE .)
#include(GNUInstallDirs)
#install(TARGETS libccs
#    LIBRARY_DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

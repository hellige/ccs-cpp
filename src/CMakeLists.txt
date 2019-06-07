option(LTO "Use link-time optimization" OFF)

if (LTO)
    message("Enabling link-time optimization")
    include(CheckIPOSupported)
    check_ipo_supported()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif ()

if (COVERAGE)
    add_compile_options(--coverage -O0)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

set(CCS_SOURCE_FILES
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

add_library(ccs_obj OBJECT ${CCS_SOURCE_FILES})
target_include_directories(ccs_obj PRIVATE .)
#set_target_properties(ccs_obj PROPERTIES PUBLIC_HEADER ../api)
set_property(TARGET ccs_obj PROPERTY POSITION_INDEPENDENT_CODE TRUE)

add_library(ccs STATIC $<TARGET_OBJECTS:ccs_obj>)
add_library(ccs::ccs ALIAS ccs)
#set_target_properties(ccs PROPERTIES PUBLIC_HEADER ../api)

add_library(ccs_so SHARED $<TARGET_OBJECTS:ccs_obj>)
add_library(ccs::ccs_so ALIAS ccs_so)
#set_target_properties(ccs_so PROPERTIES PUBLIC_HEADER ../api)

install(TARGETS ccs ccs_so EXPORT ${PROJECT_NAME}Config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# export for build tree.
export(
    TARGETS ccs ccs_so
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
)
# export for install.
install(
  EXPORT ${PROJECT_NAME}Config
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/
)

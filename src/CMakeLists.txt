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
target_include_directories(ccs_obj PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
)
set_property(TARGET ccs_obj PROPERTY POSITION_INDEPENDENT_CODE TRUE)

add_library(ccs STATIC $<TARGET_OBJECTS:ccs_obj>)
add_library(Ccs::ccs ALIAS ccs)
target_include_directories(ccs PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
)

add_library(ccs_so SHARED $<TARGET_OBJECTS:ccs_obj>)
add_library(Ccs::ccs_so ALIAS ccs_so)
target_include_directories(ccs_so PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
)
set_target_properties(ccs_so PROPERTIES OUTPUT_NAME ccs VERSION ${PROJECT_VERSION})

install(TARGETS ccs ccs_so EXPORT ${PROJECT_NAME}Config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ../api/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/
)

export(
    TARGETS ccs ccs_so
    NAMESPACE ${PROJECT_NAME}::
    FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
)

install(
  EXPORT ${PROJECT_NAME}Config
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/
)

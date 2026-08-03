cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED NAME)
    message(FATAL_ERROR "NAME is required")
endif()

if(NOT DEFINED VERSION)
    message(FATAL_ERROR "VERSION is required")
endif()

set(TARGET_NAME tbaricault_${NAME})

project(${TARGET_NAME}
    VERSION "${VERSION}"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED PACKAGES)
    set(PACKAGES "")
endif()

if(NOT DEFINED TARGETS)
    set(TARGETS "")
endif()

if(NOT DEFINED SRC)
    set(SRC "")
endif()

if(ENV)
    set(
        CMAKE_INSTALL_PREFIX
        "${ENV}"
        CACHE PATH "Install prefix"
        FORCE
    )

    list(APPEND CMAKE_PREFIX_PATH "${ENV}")
endif()

separate_arguments(PACKAGES)
separate_arguments(TARGETS)
separate_arguments(SRC)

if(SRC)
    list(TRANSFORM SRC REPLACE "^(.+)$" "src/\\1.cpp")

    add_library(${TARGET_NAME} STATIC
        ${SRC}
    )

    target_include_directories(${TARGET_NAME}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
else()
    add_library(${TARGET_NAME} INTERFACE)

    target_include_directories(${TARGET_NAME}
        INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )
endif()

set_target_properties(${TARGET_NAME} PROPERTIES
    OUTPUT_NAME "${TARGET_NAME}"
)

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
        EXPORT_NAME ${NAME}
)

foreach(PACKAGE ${PACKAGES})
    find_package(${PACKAGE} REQUIRED)
endforeach()

if(SRC)
    target_link_libraries(${TARGET_NAME}
        PUBLIC
            ${TARGETS}
    )
else()
    target_link_libraries(${TARGET_NAME}
        INTERFACE
            ${TARGETS}
    )
endif()

add_library(tbaricault::${NAME} ALIAS ${TARGET_NAME})

install(
    DIRECTORY include/
    DESTINATION include/tbaricault
)

install(
    TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}-targets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    EXPORT ${TARGET_NAME}-targets
    FILE ${TARGET_NAME}-targets.cmake
    NAMESPACE tbaricault::
    DESTINATION lib/cmake/${TARGET_NAME}
)

include(CMakePackageConfigHelpers)

set(DEPENDENCIES "${PACKAGES}")

configure_package_config_file(
    cmake/config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake/${TARGET_NAME}
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}Config.cmake
    DESTINATION
        lib/cmake/${TARGET_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}ConfigVersion.cmake
    DESTINATION
        lib/cmake/${TARGET_NAME}
)

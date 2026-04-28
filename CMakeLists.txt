cmake_minimum_required(VERSION 3.20)

project(sigs VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CPM)

#Set Version
message(STATUS "Compiling Version: ${${PROJECT_NAME}_VERSION} of project ${PROJECT_NAME}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "export the build rules into a JSON files for clang-tidy" FORCE)

#If the library is interface only use the keywork INTERFACE. If the library contain cpp files, use STATIC
add_library(${PROJECT_NAME} INTERFACE)
#adding a namespace for the library
add_library(netromdk::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        INTERFACE
        sigs.h
)

target_include_directories( ${PROJECT_NAME}
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

get_target_property(_TARGET_TYPE ${PROJECT_NAME} TYPE)
if(_TARGET_TYPE STREQUAL "INTERFACE_LIBRARY")
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERFACE_VERSION ${PROJECT_VERSION} )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION} )
endif()


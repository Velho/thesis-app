cmake_minimum_required (VERSION 3.12)
project (thesis-app)

# export the compile commands as quality of life
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (THESIS_APP_SOURCES
    src/main.c)

add_executable (thesis-app
    ${THESIS_APP_SOURCES})

# glob all the mongoose sources to be part of the compilation unit
file (GLOB MONGOOSE_SOURCES
    mongoose/src/*.h
    mongoose/src/*.c)

add_library (mongoose   ${MONGOOSE_SOURCES})

# build thesis-lib as lib without any linkage.
set (THESIS_BUILD_AS_LIBRARY TRUE CACHE BOOL "Disable Thesis built examples.")
# thesis-app builds it's own mongoose, disable mongoose building for thesis lib
set (THESIS_BUILD_MONGOOSE FALSE CACHE BOOL "Disable Thesis built mongoose.")
# thesis-lib subfolder name, required otherwise 'thesis' is used by default if undefined.
set (THESIS_PROJECT_FOLDER_NAME "thesis" CACHE STRING "Set the thesis project folder name.")

# configure thesis library without mongoose
add_subdirectory (thesis)

# copy the test certificates from tests folder in thesis
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thesis/cmake")

include (certificates)
thesis_copy_certificates (thesis-app)

## specify target args ##

target_compile_definitions (mongoose
    PUBLIC
        thesis-tls)

target_include_directories(mongoose
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mongoose>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mongoose/src>
)

target_include_directories (thesis-app
    PUBLIC
        thesis-tls mongoose)

target_link_libraries (mongoose
    PRIVATE thesis-tls)

target_link_libraries (thesis-app
    PUBLIC 
        mongoose
        thesis-tls)


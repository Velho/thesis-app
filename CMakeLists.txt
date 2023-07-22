cmake_minimum_required (VERSION 3.12)
project (thesis-app)

# export the compile commands as quality of life
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_BUILD_TYPE Debug)

option (THESIS_BUILD_MBEDTLS "Build using MBedTLS (disables thesis tls)" OFF)

message ("Building MBedTLS : ${THESIS_BUILD_MBEDTLS}")

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
set (THESIS_BUILD_MONGOOSE OFF CACHE BOOL "Disable Thesis built mongoose.")
# thesis-lib subfolder name, required otherwise 'thesis' is used by default if undefined.
set (THESIS_PROJECT_FOLDER_NAME "thesis" CACHE STRING "Set the thesis project folder name.")

# THESIS_COMPILE_DEFINITIONS is used to set compile definitions
# as such can be used to disable compilation of the thesis tls
# in case where mbedtls or openssl is used. openssl is not supported.
set (THESIS_COMPILE_DEFINITONS)
set (THESIS_MBEDTLS_DEBUG TRUE) # enable debug logs

if (THESIS_BUILD_MBEDTLS)
    if (THESIS_MBEDTLS_TLS13)
        list (APPEND THESIS_COMPILE_DEFINITIONS MBEDTLS_SSL_PROTO_TLS1_3 )
    elseif (THESIS_MBEDTLS_DEBUG)
        list (APPEND THESIS_COMPILE_DEFINITIONS MBEDTLS_DEBUG_C)
    endif ()
    list (APPEND THESIS_COMPILE_DEFINITIONS MG_ENABLE_MBEDTLS)
else ()
    list (APPEND THESIS_COMPILE_DEFINITONS MG_ENABLE_TLSE MG_ENABLE_CUSTOM_TLS)
endif ()

set (MBEDTLS_PATH mbedtls)

if (THESIS_BUILD_MBEDTLS)
    set (MBEDTLS_USER_CONFIG_FILE "${CMAKE_SOURCE_DIR}/${MBEDTLS_PATH}/tests/configs/tls13-only.h")
    add_subdirectory (${MBEDTLS_PATH})
else ()
    # configure thesis library without mongoose
    add_subdirectory (thesis)
endif ()

# copy the test certificates from tests folder in thesis
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/thesis/cmake")

include (certificates)
thesis_copy_certificates (thesis-app)

## specify target args ##

# target_compile_definitions (mongoose
#     PUBLIC
#         thesis-tls)

target_include_directories(mongoose
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/mongoose
        ${CMAKE_CURRENT_SOURCE_DIR}/mongoose/src
)

target_include_directories (thesis-app
    PUBLIC
        thesis-tls mongoose)

# does not support openssl tls backend
# target_compile_definitions (mongoose PUBLIC MG_ENABLE_MBEDTLS)

if (THESIS_BUILD_MBEDTLS)
    target_include_directories (mongoose PUBLIC ${MBEDTLS_PATH}/include)
    target_link_libraries(mongoose
        PUBLIC MbedTLS::mbedtls
               MbedTLS::mbedcrypto
               MbedTLS::mbedx509)

    # enuf adding to thesis-app and not directly to mbed targets?
    # target_compile_definitions (mongoose PUBLIC MBEDTLS_DEBUG_C)
    target_include_directories (thesis-app PUBLIC ${MBEDTLS_PATH}/include)
else ()
    # configure mongoose with thesis-tls backend
    target_link_libraries (mongoose
        PRIVATE thesis-tls)
    target_link_libraries(thesis-app
        PUBLIC thesis-tls)
endif ()

# inherited defs ??
target_compile_definitions (thesis-app PUBLIC ${THESIS_COMPILE_DEFINITIONS})
target_compile_definitions (mongoose PUBLIC ${THESIS_COMPILE_DEFINITIONS})

target_link_libraries (thesis-app PUBLIC mongoose)


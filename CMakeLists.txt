cmake_minimum_required (VERSION 3.12)
project (thesis-app)

# enable compile commands for clangd lsp
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Debug)
endif ()



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
## THESIS_BUILD_MBEDTLS option used to build tls backend against mbedtls
set (THESIS_BUILD_MBEDTLS FALSE CACHE BOOL "Build using MBedTLS (disables thesis tls)")


# THESIS_COMPILE_DEFINITIONS is used to set compile definitions
# as such can be used to disable compilation of the thesis tls
# in case where mbedtls or openssl is used. openssl is not supported.
set (THESIS_COMPILE_DEFINITIONS)

message ("Building MBedTLS : ${THESIS_BUILD_MBEDTLS}")

## Thesis mbedtls project configuration
option (THESIS_MBEDTLS_TLS13 FALSE) # enable tls 1.3 by default 
option (THESIS_MBEDTLS_DEBUG TRUE)  # enable debug logs
option (THESIS_MBEDTLS_USE_USERCONFIG FALSE) # mbedtls support built-in config from user header

set (THESIS_MBEDTLS_FOLDER mbedtls)

if (THESIS_BUILD_MBEDTLS STREQUAL TRUE)
    if (THESIS_MBEDTLS_TLS13)
        message ("mbedtls : TLS1.3")
        list (APPEND THESIS_COMPILE_DEFINITIONS MBEDTLS_SSL_PROTO_TLS1_3 MBEDTLS_SSL_TLS_C)
    endif ()
    if (THESIS_MBEDTLS_DEBUG)
        # mbedtls debug flag MBEDTLS_DEBUG_C
        list (APPEND THESIS_COMPILE_DEFINITIONS MG_MBEDTLS_DEBUG_LEVEL)
    endif ()
    # adds default tls13 config when user config is enabled
    if (THESIS_MBEDTLS_USE_USERCONFIG)
        set (MBEDTLS_USER_CONFIG_FILE "${CMAKE_SOURCE_DIR}/${THESIS_MBEDTLS_FOLDER}/tests/configs/tls13-only.h")
    endif ()

    list (APPEND THESIS_COMPILE_DEFINITIONS MG_ENABLE_MBEDTLS) # enable mbedtls
else ()
    list (APPEND THESIS_COMPILE_DEFINITIONS MG_ENABLE_TLSE MG_ENABLE_CUSTOM_TLS) # enable tlse
endif ()

## add subdirectories for different tls implementations ##
## THESIS_BUILD_MBEDTLS used to enable mbedtls          ##
## THESIS_BUILD_MBEDTLS TRUE to build mbedtls           ##
## THESIS_BUILD_MBEDTLS FALSE to build tlse             ##

# TODO fix the mbedtls build

if (THESIS_BUILD_MBEDTLS)
    add_subdirectory (${THESIS_MBEDTLS_FOLDER})
else ()
    # thesis exposes set of configuration variables to be used.
    # these variables uses same prefix THESIS_###
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

if (THESIS_BUILD_MBEDTLS)
    target_include_directories (mongoose PUBLIC ${THESIS_MBEDTLS_FOLDER}/include)
    target_link_libraries(mongoose
        PUBLIC MbedTLS::mbedtls
               MbedTLS::mbedcrypto
               MbedTLS::mbedx509)

    target_include_directories (thesis-app PUBLIC ${THESIS_MBEDTLS_FOLDER}/include)
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


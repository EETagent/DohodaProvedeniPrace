cmake_minimum_required(VERSION 3.0)
project(DohodaProvedeniPrace C)
include(ExternalProject)

# C11/17
set(CMAKE_C_STANDARD 11)

# Testy
enable_testing()
add_subdirectory (tests)

# Je potřeba zlib komprese
find_package(ZLIB REQUIRED)

# Název binárního souboru
set(EXECUTABLE_NAME dohoda_ssps)

# Optimalizovat pro velikost a strip
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -Os")

# Složka pro instalaci zkompilovaných závislostí
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

# libharu - Knihovna pro tvorbu PDF
if (CMAKE_BUILD_TYPE EQUAL "RELEASE")
    set(LIBHARU_ARGUMENTS -DCMAKE_BUILD_TYPE=Release)
else()
    set(LIBHARU_ARGUMENTS -DCMAKE_BUILD_TYPE=Debug)
endif()
ExternalProject_Add(libharu
        GIT_REPOSITORY https://github.com/libharu/libharu
        #GIT_TAG origin/devel
        CMAKE_ARGS ${LIBHARU_ARGUMENTS} -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
        )

# tomlc99 - Knihovna pro zpracování TOML
ExternalProject_Add(libtoml
        GIT_REPOSITORY https://github.com/cktan/tomlc99
        BUILD_IN_SOURCE 1
        BUILD_COMMAND COMMAND make
        INSTALL_COMMAND make prefix=${EXTERNAL_INSTALL_LOCATION} install
        DEPENDS ""
        CONFIGURE_COMMAND ""
        )

include_directories(${EXTERNAL_INSTALL_LOCATION}/include ${CMAKE_SOURCE_DIR}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

# Knihovna pro vytváření SSPŠ dohod o provedení práce
add_library(ssps_dohoda STATIC ssps_dohoda.c)
add_dependencies(ssps_dohoda libharu libtoml)


add_executable(${EXECUTABLE_NAME} main.c)
add_dependencies(${EXECUTABLE_NAME} libharu libtoml)
set(LINK_LIBRARIES
        ssps_dohoda
        hpdfs
        toml
        m
        ${ZLIB_LIBRARIES})
target_link_libraries(${EXECUTABLE_NAME} ${LINK_LIBRARIES})

#add_custom_command(
#        TARGET DohodaProvedeniPrace POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy
#        ${CMAKE_SOURCE_DIR}/fonty/LiberationSans-Regular.ttf
#        ${CMAKE_CURRENT_BINARY_DIR}/LiberationSans-Regular.ttf)

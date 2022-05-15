cmake_minimum_required(VERSION 3.21)
project(DohodaProvedeniPrace C)
include(ExternalProject)

# C23
set(CMAKE_C_STANDARD 23)

# -DBUILD_CLI=
option(BUILD_CLI "Kompilace konzolového rozhraní" ON)
# -DBUILD_GUI=
option(BUILD_GUI "Kompilace GTK4 grafického rozhraní" OFF)
# -dBUILD_TESTS=
option(BUILD_TESTS "Kompilace testů" OFF)

if (BUILD_TESTS)
    # Testy
    enable_testing()
    add_subdirectory (tests)
endif ()

# Je potřeba zlib komprese
find_package(ZLIB REQUIRED)

if (BUILD_GUI)
    # Grafické rozhraní využívá framework GTK4
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK4 REQUIRED gtk4)
endif ()

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

# Pokud je povoleno kompilování CLI rozhraní
if (BUILD_CLI)
    add_executable(${EXECUTABLE_NAME} cli/main.c)
    set(LINK_LIBRARIES
            ssps_dohoda
            hpdfs
            toml
            m
            ${ZLIB_LIBRARIES})
    target_link_libraries(${EXECUTABLE_NAME} ${LINK_LIBRARIES})
endif ()

# Pokud je povoleno kompilování GUI rozhraní
if (BUILD_GUI)
    add_executable(${EXECUTABLE_NAME}_gui gui/main.c)
    set(LINK_LIBRARIES
            ssps_dohoda
            hpdfs
            toml
            m
            ${ZLIB_LIBRARIES}
            ${GTK4_LIBRARIES})
    target_link_libraries(${EXECUTABLE_NAME}_gui ${LINK_LIBRARIES})
    target_include_directories(${EXECUTABLE_NAME}_gui PRIVATE ${GTK4_INCLUDE_DIRS})
    target_link_directories(${EXECUTABLE_NAME}_gui PRIVATE ${GTK4_LIBRARY_DIRS})
endif ()
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

find_package(ZLIB REQUIRED)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include ${CMAKE_SOURCE_DIR}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_executable (ssps_dohoda_test_hash test-hash.c)
add_executable (ssps_dohoda_test_hash-2 test-hash-2.c)
add_executable (ssps_dohoda_test_size test-size.c)


set(LINK_LIBRARIES
        ssps_dohoda
        hpdfs
        toml
        m
        ${ZLIB_LIBRARIES})

target_link_libraries(ssps_dohoda_test_hash ${LINK_LIBRARIES})
target_link_libraries(ssps_dohoda_test_hash-2 ${LINK_LIBRARIES})
target_link_libraries(ssps_dohoda_test_size ${LINK_LIBRARIES})

# Testovací dohoda
add_test (NAME PDF_SAME_HASH_TEST COMMAND ssps_dohoda_test_hash)
# Testovací dohoda s více jak 15 položkami
add_test (NAME PDF_SAME_HASH_TEST2 COMMAND ssps_dohoda_test_hash-2)
# Test velikosti souboru, musí být měnší jak 1MB
add_test (NAME PDF_SIZE_TEST COMMAND ssps_dohoda_test_size)
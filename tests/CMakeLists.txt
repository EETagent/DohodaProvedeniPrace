set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

find_package(ZLIB REQUIRED)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/tests/include )
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_executable (ssps_dohoda_test_pdf test-pdf.c)
add_executable (ssps_dohoda_test_pdf_velke test-pdf-velke.c)
add_executable (ssps_dohoda_test_razeni test-razeni.c)
add_executable (ssps_dohoda_test_size test-size.c)


set(LINK_LIBRARIES
        ssps_dohoda
        hpdfs
        toml
        m
        ${ZLIB_LIBRARIES})

target_link_libraries(ssps_dohoda_test_pdf ${LINK_LIBRARIES})
target_link_libraries(ssps_dohoda_test_pdf_velke ${LINK_LIBRARIES})
target_link_libraries(ssps_dohoda_test_razeni ${LINK_LIBRARIES})
target_link_libraries(ssps_dohoda_test_size ${LINK_LIBRARIES})

# Testovací dohoda
add_test (NAME PDF_TEST COMMAND ssps_dohoda_test_pdf)
# Testovací dohoda s více jak 15 položkami
add_test (NAME PDF_VELKE_TEST COMMAND ssps_dohoda_test_pdf_velke)
# Test zkoušící řazení v dokumentu, jinak stejný jako PDF_SAME_HASH_TEST
add_test (NAME PDF_RAZENI_TEST COMMAND ssps_dohoda_test_razeni)
# Test velikosti souboru, musí být měnší jak 1MB
add_test (NAME PDF_SIZE_TEST COMMAND ssps_dohoda_test_size)
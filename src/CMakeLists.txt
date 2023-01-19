set(SOURCES
    dec.c
    dec_deflate.c
    dec_none.c
    dec_zstd.c
    main.c
    pak.c
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${ZSTD_LIBRARIES} ${ZLIB_LIBRARIES})
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION bin)

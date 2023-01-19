cmake_minimum_required(VERSION 3.9)

project(repak LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(ZSTD IMPORTED_TARGET libzstd)
pkg_check_modules(ZLIB IMPORTED_TARGET zlib)

add_subdirectory(src)

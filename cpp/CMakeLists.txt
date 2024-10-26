cmake_minimum_required(VERSION 3.20.2)

project(crypt
  VERSION       0.1.0.0
  # HOMEPAGE_URL  "https://github.com/seanwilliamcarroll/interpreters"
)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(CTest)
include(Platform)

find_package(DocTest       2.4.11 REQUIRED)
find_package(RapidCheck           REQUIRED)
find_package(OpenSSL REQUIRED)

set(CMAKE_CXX_STANDARD            20 )
set(CMAKE_CXX_STANDARD_REQUIRED   ON )
set(CMAKE_CXX_EXTENSIONS          OFF)

add_compile_options(-Wall)
add_compile_options(-fexceptions)

add_subdirectory(crypt)
add_subdirectory(set1)
add_subdirectory(set2)

#*****************************************************************************

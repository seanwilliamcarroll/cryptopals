project(crypt VERSION 0.1.0)

add_executable(crypt-exe main.cpp)

set_target_properties(crypt-exe PROPERTIES OUTPUT_NAME crypt)

add_library(crypt-lib STATIC
  crypt.cpp)

set_target_properties(crypt-lib PROPERTIES OUTPUT_NAME crypt)

target_include_directories(crypt-lib PUBLIC inc)

target_link_libraries(crypt-lib OpenSSL::SSL)

target_link_libraries(crypt-exe crypt-lib)

if (APPLE)
  set_target_properties(crypt-exe PROPERTIES
    BUILD_RPATH "/opt/local/libexec/llvm-17/lib"
  )
endif(APPLE)

#****************************************************************************

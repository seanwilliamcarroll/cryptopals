project(crypt VERSION 0.1.0)

add_library(crypt-lib STATIC
  src/aes.cpp
  src/util.cpp
  src/raw_bytes.cpp
  src/freq_map.cpp
  src/crypt.cpp
  src/block.cpp
)

set_target_properties(crypt-lib PROPERTIES OUTPUT_NAME crypt)

target_include_directories(crypt-lib PUBLIC inc)

target_link_libraries(crypt-lib OpenSSL::SSL)

add_executable(crypt-test
  test/main.cpp
  test/raw_bytes_test.cpp)

# target_include_directories(crypt-test PUBLIC test/inc)

target_link_libraries(crypt-test
  crypt-lib
  rapidcheck
  doctest::doctest)


if (APPLE)
  set_target_properties(crypt-test PROPERTIES
    BUILD_RPATH "/opt/local/libexec/llvm-17/lib"
  )
endif (APPLE)


add_test(NAME crypt.raw_bytes COMMAND crypt-test -ts=crypt.raw_bytes)
# add_test(NAME crypt.token COMMAND crypt-test -ts=crypt.token)
# add_test(NAME crypt.lexer COMMAND crypt-test -ts=crypt.lexer)


#****************************************************************************

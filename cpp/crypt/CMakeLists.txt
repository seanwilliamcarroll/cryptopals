project(crypt VERSION 0.1.0)

add_library(crypt-lib STATIC
  src/aes.cpp
  src/util.cpp
  src/raw_bytes.cpp
  src/freq_map.cpp
)

set_target_properties(crypt-lib PROPERTIES OUTPUT_NAME crypt)

target_include_directories(crypt-lib PUBLIC inc)

target_link_libraries(crypt-lib OpenSSL::SSL)

#****************************************************************************

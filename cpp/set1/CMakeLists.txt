project(crypt VERSION 0.1.0)

add_executable(crypt-exe main.cpp)

set_target_properties(crypt-exe PROPERTIES OUTPUT_NAME crypt)

target_link_libraries(crypt-exe crypt-lib)

if (APPLE)
  set_target_properties(crypt-exe PROPERTIES
    BUILD_RPATH "/opt/local/libexec/llvm-17/lib"
  )
endif(APPLE)

#****************************************************************************

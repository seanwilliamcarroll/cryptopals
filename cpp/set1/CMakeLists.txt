project(crypt VERSION 0.1.0)

add_executable(set1-exe main.cpp)

set_target_properties(set1-exe PROPERTIES OUTPUT_NAME crypt)

target_link_libraries(set1-exe crypt-lib)

if (APPLE)
  set_target_properties(set1-exe PROPERTIES
    BUILD_RPATH "/opt/local/libexec/llvm-17/lib"
  )
endif(APPLE)

#****************************************************************************

project(crypt VERSION 0.1.0)

add_executable(set2-exe main.cpp)

set_target_properties(set2-exe PROPERTIES OUTPUT_NAME crypt)

target_link_libraries(set2-exe crypt-lib)

if (APPLE)
  set_target_properties(set2-exe PROPERTIES
    BUILD_RPATH "/opt/local/libexec/llvm-17/lib"
  )
endif(APPLE)

#****************************************************************************

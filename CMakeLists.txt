project(wnaabi CXX)
cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules") 

function(append var element)
  set(${var} "${${var}} ${element}" PARENT_SCOPE)
endfunction()

if(NOT MSVC)
  if (CYGWIN OR MINGW)
    # MinGW and Cygwin are a bit stricter and lack things like
    # 'strdup', 'stricmp', etc in c++11 mode.
    append(CMAKE_CXX_FLAGS "-std=gnu++11")
  else()
    append(CMAKE_CXX_FLAGS "-std=c++11")
  endif()
endif()

add_subdirectory(googletest)
include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/googletest/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/googlemock/include")

find_package(Sphinx REQUIRED)
sphinx_doc(doc)

find_package(PythonInterp REQUIRED)
enable_testing()

function(cxx_test name)
  add_executable(${name} "test/${name}.cpp")
  target_link_libraries(${name} gtest)
  add_test(NAME "${name}_test" COMMAND ${name} "--gtest_output='xml:${CMAKE_BINARY_DIR}'")
endfunction()

cxx_test(pretty_function)


# compile one executable with a few calls to wnaabi::get_typename
# so that python can search it and verify that names are trimmed
add_executable(trim_pretty_function "test/trim_pretty_function.cpp")
# override CMAKE_BUILD_TYPE to guarantee that it gets optimized
set_target_properties(trim_pretty_function PROPERTIES CMAKE_BUILD_TYPE Release)
add_test(NAME trim_pretty_function_test
  COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/test/trim_pretty_function.py" "$<TARGET_FILE:trim_pretty_function>")

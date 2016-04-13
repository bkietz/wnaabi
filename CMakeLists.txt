project(wnaabi CXX)
cmake_minimum_required(VERSION 3.0)

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

enable_testing()

function(cxx_test name)
  add_executable(${name} "test/${name}.cpp")
  target_link_libraries(${name} gtest)
  add_test(NAME "${name}_test" COMMAND "${name} --gtest_output='xml:${CMAKE_BINARY_DIR}'")
endfunction()

cxx_test(pretty_function)


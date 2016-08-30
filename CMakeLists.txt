project(wnaabi CXX)
cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/scripts") 
function(append var element)
  set(${var} "${${var}} ${element}" PARENT_SCOPE)
endfunction()

set(CXX_STANDARD_REQUIRED ON)
if(NOT MSVC)
  if (CYGWIN OR MINGW)
    # MinGW and Cygwin are a bit stricter and lack things
    # like 'strdup', 'stricmp', etc in c++11 mode.
    append(CMAKE_CXX_FLAGS "-std=gnu++11")
  else()
    append(CMAKE_CXX_FLAGS "-std=c++11")
  endif()
endif()

macro(slice var index junk)
  if(${index} EQUAL 1)
    set(${var} ${ARGN})
  else()
    math(EXPR next "${index}-1")
    slice(${var} ${next} ${ARGN})
  endif()
endmacro()

function(require_cxx11 target)
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
endfunction()

function(static_runtime target)
  if(MSVC)
    target_compile_options(${target}
      PRIVATE "/MT$<$<CONFIG:Debug>:d>")
  endif()
endfunction()

add_subdirectory(googletest)
include_directories(
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/googletest/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/googlemock/include")

option(WNAABI_BUILD_DOC
  "add build targets for wnaabi documentation" ON)
if(WNAABI_BUILD_DOC)
  find_package(Sphinx REQUIRED)
  sphinx_doc(documentation "doc/")
endif()

find_package(PythonInterp REQUIRED)
enable_testing()

function(cxx_test name)
  add_executable(${name} "test/${name}.cpp")
  require_cxx11(${name})
  static_runtime(${name})
  target_link_libraries(${name} gtest)
  add_test(NAME "${name}_test"
    COMMAND ${name}
    "--gtest_output='xml:${CMAKE_BINARY_DIR}'")
endfunction()

function(py_test name)
  slice(extra_args 1 ${ARGN})
  add_test(NAME "${name}_test"
    COMMAND ${PYTHON_EXECUTABLE}
    "${CMAKE_CURRENT_SOURCE_DIR}/test/${name}.py"
    "$<TARGET_FILE:trim_pretty_function>")
    #${extra_args})
endfunction()

cxx_test(pretty_function)
cxx_test(type_info)

# compile one executable with a few calls to wnaabi::get_typename
# so that python can search it and verify that names are trimmed
add_executable(trim_pretty_function "test/trim_pretty_function.cpp")
require_cxx11(trim_pretty_function)
# override CMAKE_BUILD_TYPE to guarantee that it gets optimized
set_target_properties(trim_pretty_function PROPERTIES CMAKE_BUILD_TYPE Release)
py_test(trim_pretty_function "$<TARGET_FILE:trim_pretty_function>")

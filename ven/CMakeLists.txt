cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME ven)

project(${PROJECT_NAME})

# visual studio filter
set(_SUB_SRC_DIR "${ven_SOURCE_DIR}/include")
file(GLOB_RECURSE _SUB_SRC_LIST
  LIST_DIRECTORIES false
  "${_SUB_SRC_DIR}/*.h"
  "${_SUB_SRC_DIR}/*.hpp"
  "${_SUB_SRC_DIR}/*.cpp"
)

foreach(_SUB_SRC IN ITEMS ${_SUB_SRC_LIST})
  get_filename_component(_SUB_SRC_PATH "${_SUB_SRC}" PATH)
  file(RELATIVE_PATH _SUB_SRC_REL_PATH "${_SUB_SRC_DIR}" "${_SUB_SRC_PATH}")
  string(REPLACE "/" "\\" _GROUP_PATH "${_SUB_SRC_REL_PATH}")
  source_group("${_GROUP_PATH}" FILES "${_SUB_SRC}")
endforeach()



set(SRC_LIST main.cpp stdafx.cpp stdafx.h targetver.h)

# avoid SourceFiles, HeaderFiles
source_group("" FILES ${SRC_LIST})

add_executable(${PROJECT_NAME} ${SRC_LIST} ${_SUB_SRC_LIST})

add_precompiled_header(${PROJECT_NAME} stdafx.h SOURCE_CXX stdafx.cpp FORCEINCLUDE)

cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME ven_echo_udp_server)

project(${PROJECT_NAME})

set(SRC_LIST ${PROJECT_NAME}.cpp stdafx.cpp stdafx.h targetver.h)

# avoid SourceFiles, HeaderFiles
source_group("" FILES ${SRC_LIST})

add_executable(${PROJECT_NAME} ${SRC_LIST})

add_precompiled_header(${PROJECT_NAME} stdafx.h SOURCE_CXX stdafx.cpp FORCEINCLUDE)

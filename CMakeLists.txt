#
# 64bit> cmake -G"Visual Studio 15 2017 Win64" ..
#

cmake_minimum_required(VERSION 3.5)

project(ven)

include(compile/cmake/PrecompiledHeader.cmake)

set(CMAKE_VERBOSE_MAKEFILE true)

# remove ZERO_CHECK
#set(CMAKE_SUPPRESS_REGENERATION true)

# remove RelWithDebInfo,  MinSizeRel
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)



if (MSVC)
# use precompiled header(project)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Yu")

# use /MT, /MTd
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

# use unicode characterset
add_definitions(-DUNICODE -D_UNICODE)

# create pdb
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DEBUG /OPT:REF /OPT:ICF")

else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()


include_directories(ven/include)

add_subdirectory(ven)
add_subdirectory(ven-example/ven_echo_tcp_client)
add_subdirectory(ven-example/ven_echo_tcp_server)
add_subdirectory(ven-example/ven_echo_udp_client)
add_subdirectory(ven-example/ven_echo_udp_server)

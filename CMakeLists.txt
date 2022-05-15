cmake_minimum_required(VERSION 3.22)
project(Zadanie02_client)

set(CMAKE_CXX_STANDARD 20)
#set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Werror -O2")
set(CMAKE_CXX_FLAGS "-pthread -Wall -Wextra -Wconversion -O2")

SET(Boost_USE_STATIC_LIBS FALSE)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system program_options)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF (Boost_FOUND)

SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(robots-client Buffer.cpp ClientParameters.cpp Client.cpp GameInfo.cpp)

TARGET_LINK_LIBRARIES(robots-client ${USED_LIBS})

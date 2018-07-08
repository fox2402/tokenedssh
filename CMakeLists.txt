cmake_minimum_required(VERSION 3.5.1)
project(tokenedssh-server)

enable_testing()

#package depency

#set the flags for g++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0 -O3")


#tells Cmake to use this folder for includes
include_directories(include)

#list all the files that should not be tested (like main)
set (MAIN 
     src/main.cc)

#list all the source files except main
set (SRCS 
     src/server.cc)

#list all the headers files
set (HDRS
     include/server.h)

#create a rule to make
add_executable(tkn_ssh ${SRCS} ${MAIN} ${HDRS})

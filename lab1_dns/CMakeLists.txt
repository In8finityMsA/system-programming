cmake_minimum_required(VERSION 3.20)
project(DNS)

set(CMAKE_CXX_STANDARD 17)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/hosts
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(DNS main.c dns.c dns.h hashtable.h bucket.h hashtable.c bucket.c)

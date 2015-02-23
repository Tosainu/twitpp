cmake_minimum_required(VERSION 3.1)

project(twitpp)

add_definitions(-std=c++11 -Wall -Wextra)
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,now,-z,relro")

find_package(Boost 1.55.0 COMPONENTS system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

add_executable(twitpp
  ./net/async_client.cc
  ./net/client.cc
  ./oauth/account.cc
  ./oauth/oauth.cc
  ./util/util.cc
  ./example.cc
  )

target_link_libraries(twitpp ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} pthread)

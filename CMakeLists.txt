cmake_minimum_required(VERSION 2.8)

project(twitpp)

add_definitions(-std=c++11 -Wall -Wextra)

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")
set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,now,-z,relro")

find_package(Boost 1.55.0 COMPONENTS system REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_USE_PTHREAD_INIT 1)

add_library(${PROJECT_NAME} SHARED
  net/async_client.cc
  net/client.cc
  oauth/account.cc
  oauth/oauth.cc
  util/util.cc
)

set_property(
  TARGET ${PROJECT_NAME}
  PROPERTY PUBLIC_HEADER
  net/async_client.h
  net/client.h
  net/method.h
  net/response.h
  oauth/account.h
  oauth/oauth.h
  util/util.h
)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  crypto
)

install(TARGETS ${PROJECT_NAME}
  ARCHIVE       DESTINATION lib
  LIBRARY       DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${PROJECT_NAME}
)

#example

add_executable(example example.cc)

target_link_libraries(example
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  crypto
)

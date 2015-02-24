cmake_minimum_required(VERSION 2.8)

project(twitpp)

add_definitions(-std=c++11 -Wall -Wextra)

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,now,-z,relro")

find_package(Boost 1.55.0 COMPONENTS system REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(${PROJECT_NAME} SHARED
  net/async_client.cc
  net/client.cc
  oauth/account.cc
  oauth/oauth.cc
  util/util.cc
)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  crypto
  pthread
)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(FILES
  net/async_client.h
  net/client.h
  net/method.h
  net/response.h
  oauth/account.h
  oauth/oauth.h
  util/util.h
  DESTINATION include/${PROJECT_NAME}
)

#example

add_executable(example example.cc)

target_link_libraries(example
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  crypto
  pthread
)
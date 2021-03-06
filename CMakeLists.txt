cmake_minimum_required(VERSION 2.8)

project(twitpp)

add_definitions(-std=c++11 -Wall -Wextra)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3")

if(APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl")
  set(CMAKE_MACOSX_RPATH 0)
else()
   set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-z,now,-z,relro")
endif()

find_package(Boost 1.55.0 COMPONENTS system REQUIRED)
find_package(OpenSSL 1.0.1 REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_USE_PTHREAD_INIT 1)

file(GLOB_RECURSE SOURCES src/twitpp/*.cc)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${OPENSSL_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ${PROJECT_NAME}
  LIBRARY         DESTINATION lib
)

install(DIRECTORY src/twitpp/
  DESTINATION     include/${PROJECT_NAME}
  FILES_MATCHING  PATTERN "*.h"
)

add_subdirectory(example)

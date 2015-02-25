include_directories(${PROJECT_SOURCE_DIR})

add_executable(example example.cc)
TARGET_LINK_LIBRARIES(example ${PROJECT_NAME})

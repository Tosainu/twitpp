include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(example example.cc)
TARGET_LINK_LIBRARIES(example ${PROJECT_NAME})

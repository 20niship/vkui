add_executable(main main.cpp)
message(${ALL_LINKS})
target_link_libraries(main ${ALL_LINKS})

add_executable(widget_test widget_test.cpp)
target_link_libraries(widget_test ${ALL_LINKS})


add_executable(simple gl_simple.cpp)
target_link_libraries(simple ${ALL_LINKS})

# get_filename_component(PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
# file(GLOB SHADER_FILES ${PARENT_DIR}/shader)
# file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shader)


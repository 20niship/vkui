find_package(Vulkan)
IF (NOT Vulkan_FOUND)
    message("Vulkanが見つからなかったのでvkuiのビルドはしません。")
    return()
  find_library(Vulkan_LIBRARY PROJECT_NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
  IF (Vulkan_LIBRARY)
    set(Vulkan_FOUND ON)
    MESSAGE("Using bundled Vulkan library version")
  ELSE()
    message("Vulkanが見つからなかったのでvkuiのビルドはしません。")
    return()
  ENDIF()
ENDIF()
find_package(Threads REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenCV )

include_directories(
  ${ODE_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} 
  ${realsense2_INCLUDE_DIR}
  ${OpenCL_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/vkui/3rdparty/include
  ${CMAKE_SOURCE_DIR}/vkui/include
  ${FREETYPE_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIRS}
)

file(GLOB VKUI_SRC_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/widget/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/widget/*.hpp
  ${CMAKE_SOURCE_DIR}/include/*.hpp
)
add_library(vkengine ${VKUI_SRC_FILES})

target_link_libraries(vkengine PUBLIC 
  glfw
  OpenGL::GL
  ${Vulkan_LIBRARIES}
  ${CMAKE_DL_LIBS}
  ${FREETYPE_LIBRARIES}
  ${GLFW_LIBRARY_DIRS}
  ${CMAKE_DL_LIBS}
)

set(ALL_LINKS ${ALL_LINKS} vkengine) 

file(GLOB DATA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/*)
file(COPY ${DATA_FILES} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/..")

set(EXECUTABLE_OUTPUT_PATH_DEFAULT "${EXECUTABLE_OUTPUT_PATH}")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/vkui/")
add_subdirectory(examples)
set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH_DEFAULT}")

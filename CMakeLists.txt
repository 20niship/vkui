cmake_minimum_required(VERSION 3.1)
project(vkui)

set(CMAKE_EXPORT_COMPILE_COMMANDS "on") # compile_commands.json作成

#コンパイラーがC++17に対応しているかチェック
include(CheckCXXCompilerFlag)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)
check_cxx_compiler_flag("-std=c++2a" COMPILER_SUPPORTS_CXX2A)
if(${COMPILER_SUPPORTS_CXX20})
  set(CMAKE_CXX_FLAGS "-std=c++20 ${CMAKE_CXX_FLAGS}")
elseif(${COMPILER_SUPPORTS_CXX2A})
  set(CMAKE_CXX_FLAGS "-std=c++2a ${CMAKE_CXX_FLAGS}")
else()
  message(FATAL_ERROR "${CMAKE_CXX_COMPILER} はC++20をサポートしてません。C++20に対応したコンパイラを指定してください。\n")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message("Releaseモードでビルドします")
  set(CMAKE_CXX_FLAGS "-O3 -DNDEBUG ${CMAKE_CXX_FLAGS}")
else()
  message("Debugモードでビルドします")
  set(CMAKE_CXX_FLAGS "-O2 -g ${CMAKE_CXX_FLAGS}")
endif()

message("Compiler:\n\t${CMAKE_CXX_COMPILER}")
message("compiler flags:\n\t${CMAKE_CXX_FLAGS}")


option(BUILD_DOC "Build documentation" ON)
if(BUILD_DOC)
  message("Doxygenを用いてドキュメントを生成します。")
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
      set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
      set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
      configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
      message("Doxygen build started")
      add_custom_target(doc ALL
          COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
          COMMENT "Generating API documentation with Doxygen"
          VERBATIM )
  else (DOXYGEN_FOUND)
   message("Doxygenが見つかりませんでした。このメッセージをOFFにするには、cmakeのBUILD_DOCオプションをオフにしてください")
  endif (DOXYGEN_FOUND)
endif (BUILD_DOC)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0072 NEW)
endif()


find_package(Threads REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenCV )

SET(RENDERER "WITH_NONE")

option(SET_RENDERER_VULKAN "Vulkanを使用してレンダリングを行います" ON)

if(SET_RENDERER_VULKAN) 
  find_package(Vulkan)
  if(Vulkan_FOUND)
    set(RENDERER "WITH_VULKAN" )
  endif(Vulkan_FOUND)
else()
  find_package(OpenGL)
  find_package(GLEW)
  if(OpenGL_FOUND AND GLEW_FOUND)
    set(RENDERER "WITH_OPENGL")
  endif()
endif()


if(NOT RENDERER STREQUAL "WITH_NONE")
  message("Renderer = ${RENDERER}")
else()
  message(FATAL_ERROR "VulkanをOpenGLのどちらも見つかりませんでした。Cmakeを終了します")
endif()

add_compile_definitions(${RENDERER})

include(FetchContent)
FetchContent_Declare(
  stb 
  GIT_REPOSITORY  https://github.com/nothings/stb.git
  GIT_TAG        origin/master
)
FetchContent_Declare(
  cutil 
  GIT_REPOSITORY  https://github.com/20niship/cutil.git
  GIT_TAG        origin/master
)
FetchContent_MakeAvailable(stb cutil)

include_directories(
  ${OpenCV_INCLUDE_DIRS} 
  ${FREETYPE_INCLUDE_DIRS}
  ${Vulkan_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${stb_INCLUDE_DIRS}
  ${stb_SOURCE_DIR}
  ${cutil_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/IconFontCppHeaders
  ${CMAKE_SOURCE_DIR}/include
)

file(GLOB VKUI_SRC_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/widget/*.cpp 
)

file(GLOB GL_SRC_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gl/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gl/*.hpp 
)

file(GLOB VK_SRC_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vk/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vk/*.hpp 
)


if(RENDERER STREQUAL "WITH_VULKAN")
  set(VKUI_SRC_FILES ${VKUI_SRC_FILES} ${VK_SRC_FILES})
else()
  set(VKUI_SRC_FILES ${VKUI_SRC_FILES} ${GL_SRC_FILES})
endif()

add_library(vkui ${VKUI_SRC_FILES})

set(ALL_LINKS 
  glfw
  ${GLFW_LIBRARY_DIRS}
  ${CMAKE_DL_LIBS}
  ${FREETYPE_LIBRARIES}
)

if(RENDERER STREQUAL "WITH_VULKAN")
  set(ALL_LINKS ${ALL_LINKS} ${Vulkan_LIBRARIES})
else()
  set(ALL_LINKS ${ALL_LINKS} OpenGL::GL ${GLEW_LIBRARIES})
endif()

target_link_libraries(vkui ${ALL_LINKS})

set(ALL_LINKS ${ALL_LINKS} vkui)

file(GLOB DATA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/*)
file(COPY ${DATA_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(examples)

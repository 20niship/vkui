cmake_minimum_required(VERSION 3.1)

project(vkui)

set(CMAKE_EXPORT_COMPILE_COMMANDS "on") # compile_commands.json作成

if (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  message( FATAL_ERROR "Intel IOCのビルドにはまだ対応していないよ")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message( FATAL_ERROR "Windowsのビルドにはまだ対応していないよ")
endif()

#コンパイラーがC++17に対応しているかチェック
include(CheckCXXCompilerFlag)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)

check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)
check_cxx_compiler_flag("-std=c++2a" COMPILER_SUPPORTS_CXX2A)

# if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") # NOTE: （stdlibc++でコンパイルされた）protobuf を使うと libc++ は使えない...
#   # set(CMAKE_CXX_COMPILER "clang++")
#   set(CMAKE_CXX_FLAGS "-stdlib=libc++ ${CMAKE_CXX_FLAGS}")
#   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
# endif()

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

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif()

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

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/stb)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/IconFontCppHeaders)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/cutil/include)

file(GLOB VKUI_SRC_FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/widget/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/vk/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/gl/*.cpp 
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/widget/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/vk/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gl/*.hpp
  ${CMAKE_SOURCE_DIR}/include/*.hpp
)
add_library(vkui ${VKUI_SRC_FILES})

set(ALL_LINKS 
  glfw
  OpenGL::GL
  ${CMAKE_DL_LIBS}
  ${FREETYPE_LIBRARIES}
  ${GLFW_LIBRARY_DIRS}
  ${Vulkan_LIBRARIES}
  ${FREETYPE_LIBRARIES}
)

target_link_libraries(vkui ${ALL_LINKS})

set(ALL_LINKS ${ALL_LINKS} vkui)

file(GLOB DATA_FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/*)
file(COPY ${DATA_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# set(EXECUTABLE_OUTPUT_PATH_DEFAULT "${EXECUTABLE_OUTPUT_PATH}")
# set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/examples/")
add_subdirectory(examples)
# set(EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH_DEFAULT}")

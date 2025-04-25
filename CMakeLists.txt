cmake_minimum_required (VERSION 3.15)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("FinalProject")

# Add some add_dependencies
include_directories(
  ${CMAKE_SOURCE_DIR}/include/Eigen/include
  ${CMAKE_SOURCE_DIR}/include/glad/include
  ${CMAKE_SOURCE_DIR}/include/GLFW/include
  ${CMAKE_SOURCE_DIR}/include/tinyply/include
  ${CMAKE_SOURCE_DIR}/include/json/include
)

# compile glad
add_library(glad STATIC
  ${CMAKE_SOURCE_DIR}/include/glad/src/glad.c
)
target_include_directories(glad PUBLIC
  ${CMAKE_SOURCE_DIR}/include/glad/include
)

# Add GLFW directory
include(FetchContent)
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# Add eigen and json libraries
add_library(Eigen INTERFACE)
add_library(json INTERFACE)
add_library(tinyply INTERFACE)

# 只需要把它们的 include 路径传递下去
target_include_directories(Eigen INTERFACE ${CMAKE_SOURCE_DIR}/include/Eigen)
target_include_directories(json INTERFACE ${CMAKE_SOURCE_DIR}/include/json)
target_include_directories(tinyply INTERFACE ${CMAKE_SOURCE_DIR}/include/tinyply)

find_package(OpenGL REQUIRED)

file(COPY ${CMAKE_SOURCE_DIR}/Shader DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/Config DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB_RECURSE SRCS src/*.cpp)
add_executable(FinalProject ${SRCS})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET FinalProject PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(FinalProject
  PRIVATE
    glad
    glfw
    Eigen
    tinyply
    json
    OpenGL::GL
)

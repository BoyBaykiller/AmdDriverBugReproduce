cmake_minimum_required(VERSION 3.21)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

#----------------------------------------------------------------------

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GLFW_LIBRARY_TYPE "Link glfw static or dynamic" STATIC)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.3.2
)

message("Fetching glfw")
FetchContent_MakeAvailable(glfw)

#----------------------------------------------------------------------

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
)

message("Fetching glad")
FetchContent_GetProperties(glad)

if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile")
    set(GLAD_API "gl=4.6" CACHE STRING "API type/version pairs, like \"gl=4.6\", no version means latest")
    set(GLAD_GENERATOR "c" CACHE STRING "Language to generate the bindings for")
    set(GLAD_EXTENSIONS "GL_ARB_bindless_texture" CACHE STRING "Extensions to take into consideration when generating the bindings")
    add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR})
endif()

#----------------------------------------------------------------------

project(AmdDriverBugReproduce)

set(sourceFiles BindlessCompressedLodArtifacts/main.cpp)
add_executable(BindlessCompressedLodArtifacts "BindlessCompressedLodArtifacts" ${sourceFiles})
target_link_libraries(BindlessCompressedLodArtifacts PRIVATE glad glfw)

set(sourceFiles BindlessPerformanceOverhead/main.cpp)
add_executable(BindlessPerformanceOverhead "BindlessPerformanceOverhead" ${sourceFiles})
target_link_libraries(BindlessPerformanceOverhead PRIVATE glad glfw)

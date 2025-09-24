cmake_minimum_required(VERSION 3.21)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

#----------------------------------------------------------------------

message("Fetching glfw")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GLFW_LIBRARY_TYPE "Link glfw static or dynamic" STATIC)
option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG        3.3.10
)

FetchContent_MakeAvailable(glfw)

#----------------------------------------------------------------------

message("Fetching glad")

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG        v2.0.8
    SOURCE_SUBDIR  cmake
)

FetchContent_MakeAvailable(glad)

glad_add_library(glad_gl_core_46 STATIC LANGUAGE c REPRODUCIBLE API gl:core=4.6 EXTENSIONS GL_ARB_bindless_texture)
target_include_directories(glad_gl_core_46 PUBLIC ${CMAKE_BINARY_DIR}/glad_gl_core_46/gladsources/include)

#----------------------------------------------------------------------

project(AmdDriverBugReproduce)

set(commonFiles 
    Common/Common.h
    Common/Common.cpp
)

add_executable(BindlessCompressedLodArtifacts "BindlessCompressedLodArtifacts" BindlessCompressedLodArtifacts/Main.cpp ${commonFiles})
target_link_libraries(BindlessCompressedLodArtifacts PRIVATE glad_gl_core_46 glfw)

add_executable(BindlessPerformanceOverhead "BindlessPerformanceOverhead" BindlessPerformanceOverhead/Main.cpp ${commonFiles})
target_link_libraries(BindlessPerformanceOverhead PRIVATE glad_gl_core_46 glfw)

add_executable(MappedBufferHostMemInvestigation "MappedBufferHostMemInvestigation" MappedBufferHostMemInvestigation/Main.cpp ${commonFiles})
target_link_libraries(MappedBufferHostMemInvestigation PRIVATE glad_gl_core_46 glfw)

add_executable(DispatchesMissedDependency "DispatchesMissedDependency" DispatchesMissedDependency/Main.cpp ${commonFiles})
target_link_libraries(DispatchesMissedDependency PRIVATE glad_gl_core_46 glfw)

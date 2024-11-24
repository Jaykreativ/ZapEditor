cmake_minimum_required(VERSION 3.10)

project(ZapEditor)

file(GLOB ZapEditor_SRC 
    "src/*.cpp"
    "header/*.h"
)

file(GLOB_RECURSE ZapEditor_Shader
    "Shader/*"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(ZapEditor ${ZapEditor_SRC} ${ZapEditor_Shader})

source_group("Source" FILES ${ZapEditor_SRC})
source_group("Shader" FILES ${ZapEditor_Shader})

set(${CMAKE_BUILD_PARALLEL_LEVEL} 16)
add_subdirectory("../Zap" "${PROJECT_SOURCE_DIR}/../Zap")

set(Zap_DIR "${PROJECT_SOURCE_DIR}/../Zap")

target_link_libraries(ZapEditor PUBLIC Zap)
target_include_directories(
    ZapEditor PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/header"
    "${Zap_DIR}/include"
    "${Zap_DIR}/Dependencies/VulkanFramework/include"
    "${Zap_DIR}/Dependencies/glfw/include"
    "${Zap_DIR}/Dependencies/imgui"
    "${Zap_DIR}/Dependencies/glm/glm"
    "${Zap_DIR}/Dependencies/PhysX/physx/include"
    "${Zap_DIR}/Dependencies/assimp/include"
    "$ENV{VULKAN_SDK}/Include"
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" PHYSX_BUILD_TYPE)

file(GLOB PhysX_DLLs
    "${Zap_DIR}/Dependencies/PhysX/physx/bin/win.x86_64.vc143.md/${PHYSX_BUILD_TYPE}/PhysX*.dll"
)

file(COPY ${PhysX_DLLs}
        DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}")
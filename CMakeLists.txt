cmake_minimum_required(VERSION 3.10)

project(ZapEditor)

file(GLOB ZapEditor_SRC 
    "src/*.cpp"
    "src/*.h"
)

add_executable(ZapEditor ${ZapEditor_SRC})

add_subdirectory("../Zap" "../Zap/Debug")

set(Zap_DIR "${PROJECT_SOURCE_DIR}/../Zap")

target_link_libraries(ZapEditor PUBLIC Zap)
target_include_directories(
    ZapEditor PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${Zap_DIR}/include"
    "${Zap_DIR}/Dependencies/VulkanFramework/include"
    "${Zap_DIR}/Dependencies/glfw/include"
    "${Zap_DIR}/Dependencies/imgui"
    "${Zap_DIR}/Dependencies/glm/glm"
    "${Zap_DIR}/Dependencies/PhysX/physx/include"
    "${Zap_DIR}/Dependencies/assimp/include"
    "$ENV{VULKAN_SDK}/Include"
)

file(GLOB PhysX_DLLs
    "${Zap_DIR}/Dependencies/PhysX/physx/bin/win.x86_64.vc143.mt/debug/PhysX*.dll"
)

file(COPY ${PhysX_DLLs}
        DESTINATION "${PROJECT_SOURCE_DIR}/Debug")
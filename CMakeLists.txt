cmake_minimum_required(VERSION 3.10)

project(ZapEditor)

file(GLOB ZapEditor_SRC 
    "src/*.cpp"
    "src/*.h"
)
file(GLOB ZapEditor_Layers_SRC 
    "src/Layers/*.cpp"
    "src/Layers/*.h"
)
file(GLOB ZapEditor_Handlers_SRC 
    "src/Handlers/*.cpp"
    "src/Handlers/*.h"
)
file(GLOB ZapEditor_Objects_SRC 
    "src/Objects/*.cpp"
    "src/Objects/*.h"
)

file(GLOB_RECURSE ZapEditor_Shader
    "Shader/*"
)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(ZapEditor ${ZapEditor_SRC} ${ZapEditor_Layers_SRC} ${ZapEditor_Handlers_SRC} ${ZapEditor_Objects_SRC} ${ZapEditor_Shader})

set_property(TARGET ZapEditor PROPERTY CXX_STANDARD 17)
set_property(TARGET ZapEditor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

source_group("Source" FILES ${ZapEditor_SRC})
source_group("Source/Layers" FILES ${ZapEditor_Layers_SRC})
source_group("Source/Handlers" FILES ${ZapEditor_Handlers_SRC})
source_group("Source/Objects" FILES ${ZapEditor_Objects_SRC})
source_group("Shader" FILES ${ZapEditor_Shader})

set(${CMAKE_BUILD_PARALLEL_LEVEL} 16)
add_subdirectory("../Zap" "${PROJECT_SOURCE_DIR}/../Zap")

set(Zap_DIR "${PROJECT_SOURCE_DIR}/../Zap")

target_link_libraries(ZapEditor PUBLIC Zap)
target_include_directories(
    ZapEditor PUBLIC
    Zap
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/src/Layers"
    "${PROJECT_SOURCE_DIR}/src/Handlers"
    "${PROJECT_SOURCE_DIR}/src/Objects"
)

string(TOLOWER "${CMAKE_BUILD_TYPE}" PHYSX_BUILD_TYPE)

file(GLOB PhysX_DLLs
    "${Zap_DIR}/Dependencies/PhysX/physx/bin/win.x86_64.vc143.md/${PHYSX_BUILD_TYPE}/PhysX*.dll"
)

file(COPY ${PhysX_DLLs}
        DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}")
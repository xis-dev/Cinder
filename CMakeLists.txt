cmake_minimum_required(VERSION 3.15)

project(Cinder LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(OpenGL REQUIRED)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)



FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.89.9
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.1
)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "Don't build assimp tools" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Don't build tests" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Don't install" FORCE)
FetchContent_MakeAvailable(assimp)


set (LOCAL_DEPENDENCIES_DIR ${PROJECT_SOURCE_DIR}/dependencies)

set(GLAD_DIR ${LOCAL_DEPENDENCIES_DIR}/glad)

set(GLM_DIR ${LOCAL_DEPENDENCIES_DIR}/glm)

set(STB_DIR ${LOCAL_DEPENDENCIES_DIR}/stb)

set(ALL_DEPENDENCIES_DIRS
    ${LOCAL_DEPENDENCIES_DIR}
    ${GLAD_DIR}
    ${GLM_DIR}
    ${STB_DIR}
    ${glfw_SOURCE_DIR}
    ${imgui_SOURCE_DIR}
    ${assimp_SOURCE_DIR})

set(PROJECT_INCLUDE_DIR 
    ${PROJECT_SOURCE_DIR}/includes
)

set (INCLUDE_DIRS
    ${PROJECT_INCLUDE_DIR}
    includes/Engine
    includes/Entity
    includes/Graphics
    includes/Math
    includes/Primitives
    includes/Resources
    includes/Scene
    includes/Utilities
    
)
set (HEADER_SOURCES
    includes/Graphics/Vertex.h
    includes/Primitives/Cube.h
    includes/Primitives/Plane.h
    "includes/Resources/Resource.h"
    includes/Resources/ResourceManager.h
    includes/Utilities/FileManager.h
    includes/Utilities/IconRegistry.h

    includes/Scene/Camera.h
    includes/Scene/Scene.h

)

set(PROJECT_SOURCES
     src/main.cpp

     src/Engine/Engine.cpp

     src/Entity/Entity.cpp
     src/Entity/LightEntity.cpp
     src/Entity/MeshEntity.cpp

     src/Resources/Material.cpp
     src/Resources/Mesh.cpp
     src/Resources/Shader.cpp
     src/Resources/Texture.cpp

     src/Scene/Camera.cpp
     src/Scene/Scene.cpp

     ${GLAD_DIR}/glad.c
     ${STB_DIR}/stb.cpp

     src/main.cpp

     src/Engine/Engine.cpp

     src/Entity/Entity.cpp
     src/Entity/LightEntity.cpp
     src/Entity/MeshEntity.cpp

     src/Resources/Material.cpp
     src/Resources/Mesh.cpp
     src/Resources/Shader.cpp
     src/Resources/Texture.cpp

     src/Scene/Camera.cpp
     src/Scene/Scene.cpp
)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)


add_executable(Cinder ${PROJECT_SOURCES} ${IMGUI_SOURCES} ${HEADER_SOURCES} )

target_compile_options(Cinder PRIVATE /std:c++20)

target_include_directories(Cinder PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${INCLUDE_DIRS}
        ${ALL_DEPENDENCIES_DIRS}

        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_compile_definitions(Cinder PRIVATE
        IMGUI_DEFINE_MATH_OPERATORS
        GLFW_INCLUDE_NONE
)

target_link_libraries(Cinder PRIVATE
        glfw
        assimp
        ${OPENGL_LIBRARIES}
)
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
endif()

if(WIN32)
    target_link_libraries(Cinder PRIVATE opengl32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(Cinder PRIVATE GL)
endif()
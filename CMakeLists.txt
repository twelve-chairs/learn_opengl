###################
# PRE-REQUISITES: #
###################
#   - Conan       #
#   - Cmake       #
###################

cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME "learn_opengl")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

project(${PROJECT_NAME} CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})

#### Set up Conan and build dependencies #### https://github.com/conan-io/cmake-conan ##################################
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
            "${CMAKE_BINARY_DIR}/conan.cmake"
            TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

set(CONAN_SYSREQUIRES_MODE "enabled")
conan_cmake_configure(REQUIRES
        glfw/3.3.6
        glew/2.2.0
        imgui/1.86
        glm/0.9.9.8
        stb/cci.20210910
#        assimp/5.1.6
        spdlog/1.9.2
        GENERATORS CMakeToolchain CMakeDeps)

conan_cmake_autodetect(settings)
conan_cmake_install(PATH_OR_REFERENCE .
        BUILD missing
        REMOTE conancenter
        SETTINGS ${settings})
########################################################################################################################

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(imgui REQUIRED)
find_package(glm REQUIRED)
find_package(stb REQUIRED)
find_package(assimp REQUIRED)
find_package(spdlog REQUIRED)

file(GLOB H_FILES "src/*.h")
file(GLOB CPP_FILES "src/*.cpp")
file(GLOB IMGUI "src/include/imgui/*.*")
file(GLOB INCLUDE "src/include/*.*/*.*/*.*")

add_executable(${PROJECT_NAME} ${H_FILES} ${CPP_FILES} ${IMGUI}  ${INCLUDE})

target_link_libraries(${PROJECT_NAME} PRIVATE
        GLEW::GLEW
        glfw
        imgui::imgui
        glm::glm
        stb::stb
        assimp::assimp
        spdlog::spdlog)

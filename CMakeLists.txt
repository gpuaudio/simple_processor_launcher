# Copyright (c) 2024 Braingines SA/GPU Audio INC - All Rights Reserved
# Unauthorized copying of this file is strictly prohibited
# Proprietary and confidential

cmake_minimum_required(VERSION 3.22.0 FATAL_ERROR)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Project Name (Unique project name)
set(project_name ProcLaunch)

include(FetchContent)
cmake_policy(SET CMP0135 NEW)

# List of components included in the project
set(components ProcLaunchLib gain_launcher iir_launcher fir_launcher)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Configuring languages for current project
project(${project_name} LANGUAGES C CXX)

# Enable testing
enable_testing()

set(cmake-common-${project_name}_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

list(INSERT CMAKE_MODULE_PATH 0 "${cmake-common-${project_name}_SOURCE_DIR}")

set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)

# AudioFile
FetchContent_Declare(
    AudioFile
    URL https://github.com/adamstark/AudioFile/archive/refs/tags/1.1.0.zip
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common/include/AudioFile
)
FetchContent_MakeAvailable(AudioFile)

# Process all project components. call add_subdirectory
foreach (component ${components})
    add_subdirectory(${component})
endforeach ()

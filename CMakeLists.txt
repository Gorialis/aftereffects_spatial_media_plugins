cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(aftereffects_spatial_media_plugins C CXX)


### General project settings

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE GLOBAL_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/global/*.c
    ${CMAKE_SOURCE_DIR}/global/*.cpp
)

file(GLOB_RECURSE GLOBAL_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/global/*.h
    ${CMAKE_SOURCE_DIR}/global/*.hpp
)

link_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/include)

execute_process(
    COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REV_COUNT
)

add_compile_definitions(GIT_REV_COUNT=${GIT_REV_COUNT})
message(STATUS "Version set to ${GIT_REV_COUNT}")


### After Effects SDK linking

set(AFTEREFFECTS_SDK "C:/Download/Adobe_After_Effects_CC_15.0_Win_SDK/Adobe After Effects CC 15.0 Win SDK" CACHE PATH "The folder containing the After Effects SDK. This should contain the documentation PDF and the Examples folder.")
set(AFTEREFFECTS_LOCATION "C:/Program Files/Adobe/Adobe After Effects CC 2018" CACHE PATH "The folder to your After Effects install. This folder should contain the 'Support Files' folder.")
set(OUTPUT_LOCALLY FALSE CACHE BOOL "Whether to output the .aex binary to the local (default) output path, instead of into the After Effects plugin directory. Turning this on prevents runtime debugging (as After Effects will be unable to load the plugins).")

set(AFTEREFFECTS_BINARY "${AFTEREFFECTS_LOCATION}/Support Files/AfterFX.exe")

include_directories(
    ${AFTEREFFECTS_SDK}/Examples/Headers
    ${AFTEREFFECTS_SDK}/Examples/Headers/SP
    ${AFTEREFFECTS_SDK}/Examples/Headers/Win
    ${AFTEREFFECTS_SDK}/Examples/Resources
    ${AFTEREFFECTS_SDK}/Examples/Util
)

set(AFTEREFFECTS_SOURCE_FILES
    ${AFTEREFFECTS_SDK}/Examples/Util/AEGP_SuiteHandler.cpp
    ${AFTEREFFECTS_SDK}/Examples/Util/MissingSuiteError.cpp
)

if(NOT OUTPUT_LOCALLY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${AFTEREFFECTS_LOCATION}/Support Files/Plug-ins/Spatial Media Plugins/")
endif()


### Perspective to Equirectangular

# Touch resource manifest so CMake knows we intend to generate it
add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rc"
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rc"
)

# Add generation target
add_custom_target(
    perspective_to_equirectangular_resources
    COMMAND cl.exe /I "${AFTEREFFECTS_SDK}/Examples/Headers" /EP "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.r" > "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rr"
    COMMAND "${AFTEREFFECTS_SDK}/Examples/Resources/PiPLtool.exe" "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rr" "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rrc"
    COMMAND cl.exe /D "MSWindows" /EP "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rrc" > "${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rc"
)

# Glob in project files
file(GLOB_RECURSE P2E_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/*.c
    ${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/*.cpp
)

file(GLOB_RECURSE P2E_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/*.h
    ${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/*.hpp
)

# Define library (.aex)
add_library(perspective_to_equirectangular SHARED
    ${P2E_HEADER_FILES} ${P2E_SOURCE_FILES} ${CMAKE_SOURCE_DIR}/perspective_to_equirectangular/PiPL.rc
    ${GLOBAL_HEADER_FILES} ${GLOBAL_SOURCE_FILES}
    ${AFTEREFFECTS_SOURCE_FILES}
)

# Plugin depends on PiPL resources
add_dependencies(perspective_to_equirectangular perspective_to_equirectangular_resources)

# Set library's extension to .aex (After Effects eXtension)
set_target_properties(
    perspective_to_equirectangular PROPERTIES
    SUFFIX .aex
    VS_DEBUGGER_COMMAND ${AFTEREFFECTS_BINARY}
)

### Equirectangular to Perspective

# Touch resource manifest so CMake knows we intend to generate it
add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rc"
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rc"
)

# Add generation target
add_custom_target(
    equirectangular_to_perspective_resources
    COMMAND cl.exe /I "${AFTEREFFECTS_SDK}/Examples/Headers" /EP "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.r" > "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rr"
    COMMAND "${AFTEREFFECTS_SDK}/Examples/Resources/PiPLtool.exe" "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rr" "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rrc"
    COMMAND cl.exe /D "MSWindows" /EP "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rrc" > "${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rc"
)

# Glob in project files
file(GLOB_RECURSE P2E_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/*.c
    ${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/*.cpp
)

file(GLOB_RECURSE P2E_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/*.h
    ${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/*.hpp
)

# Define library (.aex)
add_library(equirectangular_to_perspective SHARED
    ${P2E_HEADER_FILES} ${P2E_SOURCE_FILES} ${CMAKE_SOURCE_DIR}/equirectangular_to_perspective/PiPL.rc
    ${GLOBAL_HEADER_FILES} ${GLOBAL_SOURCE_FILES}
    ${AFTEREFFECTS_SOURCE_FILES}
)

# Plugin depends on PiPL resources
add_dependencies(equirectangular_to_perspective equirectangular_to_perspective_resources)

# Set library's extension to .aex (After Effects eXtension)
set_target_properties(
    equirectangular_to_perspective PROPERTIES
    SUFFIX .aex
    VS_DEBUGGER_COMMAND ${AFTEREFFECTS_BINARY}
)

### Equirectangular to Equirectangular

# Touch resource manifest so CMake knows we intend to generate it
add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rc"
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rc"
)

# Add generation target
add_custom_target(
    perspective_to_perspective_resources
    COMMAND cl.exe /I "${AFTEREFFECTS_SDK}/Examples/Headers" /EP "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.r" > "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rr"
    COMMAND "${AFTEREFFECTS_SDK}/Examples/Resources/PiPLtool.exe" "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rr" "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rrc"
    COMMAND cl.exe /D "MSWindows" /EP "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rrc" > "${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rc"
)

# Glob in project files
file(GLOB_RECURSE P2E_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/perspective_to_perspective/*.c
    ${CMAKE_SOURCE_DIR}/perspective_to_perspective/*.cpp
)

file(GLOB_RECURSE P2E_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/perspective_to_perspective/*.h
    ${CMAKE_SOURCE_DIR}/perspective_to_perspective/*.hpp
)

# Define library (.aex)
add_library(perspective_to_perspective SHARED
    ${P2E_HEADER_FILES} ${P2E_SOURCE_FILES} ${CMAKE_SOURCE_DIR}/perspective_to_perspective/PiPL.rc
    ${GLOBAL_HEADER_FILES} ${GLOBAL_SOURCE_FILES}
    ${AFTEREFFECTS_SOURCE_FILES}
)

# Plugin depends on PiPL resources
add_dependencies(perspective_to_perspective perspective_to_perspective_resources)

# Set library's extension to .aex (After Effects eXtension)
set_target_properties(
    perspective_to_perspective PROPERTIES
    SUFFIX .aex
    VS_DEBUGGER_COMMAND ${AFTEREFFECTS_BINARY}
)

### Equirectangular to Equirectangular

# Touch resource manifest so CMake knows we intend to generate it
add_custom_command(
    OUTPUT "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rc"
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rc"
)

# Add generation target
add_custom_target(
    equirectangular_to_equirectangular_resources
    COMMAND cl.exe /I "${AFTEREFFECTS_SDK}/Examples/Headers" /EP "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.r" > "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rr"
    COMMAND "${AFTEREFFECTS_SDK}/Examples/Resources/PiPLtool.exe" "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rr" "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rrc"
    COMMAND cl.exe /D "MSWindows" /EP "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rrc" > "${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rc"
)

# Glob in project files
file(GLOB_RECURSE P2E_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/*.c
    ${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/*.cpp
)

file(GLOB_RECURSE P2E_HEADER_FILES
    ${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/*.h
    ${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/*.hpp
)

# Define library (.aex)
add_library(equirectangular_to_equirectangular SHARED
    ${P2E_HEADER_FILES} ${P2E_SOURCE_FILES} ${CMAKE_SOURCE_DIR}/equirectangular_to_equirectangular/PiPL.rc
    ${GLOBAL_HEADER_FILES} ${GLOBAL_SOURCE_FILES}
    ${AFTEREFFECTS_SOURCE_FILES}
)

# Plugin depends on PiPL resources
add_dependencies(equirectangular_to_equirectangular equirectangular_to_equirectangular_resources)

# Set library's extension to .aex (After Effects eXtension)
set_target_properties(
    equirectangular_to_equirectangular PROPERTIES
    SUFFIX .aex
    VS_DEBUGGER_COMMAND ${AFTEREFFECTS_BINARY}
)

cmake_minimum_required(VERSION 3.16)

message(STATUS "[Mia] Welcome to Mia :)")


#
#  Options
#

set(MIA_PROJECT "MiaApp" CACHE STRING "Mia Project name")

# vendored version of sdl, instead of system linked (SDL3 only)
option(MIA_USE_VENDOR_SDL "Use vendored version of SDL" OFF)
option(MIA_USE_VENDOR_SDL_IMAGE "Use vendored version of SDL_image" OFF)
option(MIA_USE_VENDOR_SDL_NET "Use vendored version of SDL_net" OFF)
option(MIA_USE_VENDOR_SDL_TTF "Use vendored version of SDL_ttf" OFF)

option(MIA_USE_SDL2 "Use SDL2 (otherwise SDL3)" OFF)
option(MIA_USE_TTF "Enable TTF font rendering" OFF)
option(MIA_USE_SOCKET "Enable sockets" OFF)
option(MIA_USE_THREAD "Enable threading" ON)
option(MIA_USE_THREAD_EMSCRIPTEN "Second check to enable threading in an emscripten build" OFF)
option(MIA_USE_FETCH "Enable fetching (http(s) client)" ON)
option(MIA_USE_ZIP "Enable zip compression apis" ON)
option(MIA_USE_GAMEPAD "Enable gamepad support" ON)

option(MIA_USE_SANITIZER "Enable sanitizer for debugging" ON)
option(MIA_USE_TESTS "Enable running tests at start" ON)
option(MIA_USE_TESTS_RELEASE "Second check to enable tests in release build" OFF)
option(MIA_USE_GL_CHECK "Enable OpenGL error checks" ON)
option(MIA_USE_LOG_TRACE "Enable trace logging" ON)
option(MIA_USE_LOG_DEBUG "Enable debug logging" ON)
option(MIA_USE_FILE_WARN_NATIVE "Logs a warning on native file path" ON)
option(MIA_USE_METRICS "Enable metrics logging" ON)
option(MIA_USE_METRICS_SYS "Enable system metrics logging" OFF)
option(MIA_USE_BREADCRUMPS "Enable OObj breadcrumps" OFF)
option(MIA_USE_CPUGPU_SYNC "Enable syncing the cpu and gpu at frame end (for better load calculations, but worse performance)" ON)
option(MIA_USE_BUILT_AS_LIBRARY "Builds as a shared library (needed by Android)" OFF)

# Bundling each module with a single module source file, speeds up compilation + linking a lot
option(MIA_USE_UNITYBUILD "Use unity build for faster compilation" ON)




# Depending on the platform and configuration, the following definitions may be set:
#
# Platform list:
# MIA_PLATFORM_MACOS                Apple MacOs
# MIA_PLATFORM_LINUX                 Ubuntu, etc.
# MIA_PLATFORM_MSVC                 Microsoft C++ Compiler
# MIA_PLATFORM_MINGW                Windows Msys2 Mingw-w64 (currently experimental)
# MIA_PLATFORM_CXXDROID             Android Cxx compiler App
# MIA_PLATFORM_EMSCRIPTEN           emscripten web compiler
# MIA_PLATFORM_ANDROID              AndroidStudio
#
# Platform collections:
# MIA_PLATFORM_WINDOWS:             MIA_PLATFORM_MSVC + MIA_PLATFORM_MINGW
# MIA_PLATFORM_DESKTOP:             MIA_PLATFORM_MACOS + MIA_PLATFORM_WINDOWS + MIA_PLATFORM_LINUX
#
# MIA_OPTION_SDL2                   SDL2 fallback instad of SDL3
# MIA_OPTION_GLES                   for OpenGLES3.0 instead of OpenGL3.3
# MIA_OPTION_GLEW                   if GLEW is available
# MIA_OPTION_TTF                    for ttf text render support using SDL_ttf
# MIA_OPTION_THREAD                 to use threading stuff
# MIA_OPTION_SOCKET                 to use sockets (uses SDL_net)
# MIA_OPTION_FETCH                  to use fetch   (http rest with curl or another implementation)
# MIA_OPTION_ZIP                    to enable zip compression apis
# MIA_OPTION_GAMEPAD                loads a gamepad (game controller) if available
# MIA_OPTION_SANITIZER              use sanitizer checks for debugging
# MIA_OPTION_TESTS                  start module test in test/* within o_init (will call o_exit on failure...)
# MIA_OPTION_GL_CHECK               checks for gl errors
# MIA_OPTION_LOG_TRACE              enables trace logging (not in release)
# MIA_OPTION_LOG_DEBUG              enables debug logging (not in release)
# MIA_OPTION_LOG_VIA_SDL            logs via sdl, else stdio
# MIA_OPTION_FILE_WARN_NATIVE       logs a warning if a native file is used
# MIA_OPTION_METRICS                enables acquiring metrics
# MIA_OPTION_METRICS_SYS            adds metrics from system roots like [mia]sound
# MIA_OPTION_BREADCRUMPS            enables OObj breadcrumps
# MIA_OPTION_CPUGPU_SYNC            glFinish() call to sync at frame end (worse performance, but better load calcs)
# MIA_OPTION_TERMINALCOLOR_OFF      turn off terminal colors
# MIA_OPTION_LOG_COMPACT            disable time and file in logs
# MIA_OPTION_UNITYBUILD             to bundle a module into a single source file, speeds up linking
# MIA_OPTION_TITLE                  set to the title, else Mia
# MIA_OPTION_INIT_FULLSCREEN        if set, screen mode default is fullscreen, else window


#
# Project definition
#

message(STATUS "[Mia] Project definition")

# Includes:
# - "function(platform_detection)"
# - "function(platform_set_toolchain)"
# - "function(platform_link_sdl3)"
include(cmake/platform.cmake)


# from "include(cmake/platform.cmake)"
# automatically sets the vcpkg toolchain to "C://dev/vcpkg" (or "D://dev/vcpkg" as a fallback) on windows
# this needs to be done BEFORE defining the project
platform_set_toolchain()


message(STATUS "[Mia]   Project: ${MIA_PROJECT}")
project(${MIA_PROJECT} C)
add_definitions(-DMIA_OPTION_TITLE=${MIA_PROJECT})

set(CMAKE_C_STANDARD 11)
message(STATUS "[Mia]   Compiler: ${CMAKE_C_COMPILER_ID}")

# set default configuration
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "[Mia]   Setting build type to Debug as none was specified.")
    set(CMAKE_BUILD_TYPE Debug CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif ()
message(STATUS "[Mia]   Using build type: " ${CMAKE_BUILD_TYPE})


# used logo, may be updated from apps
set(MIA_LOGO "${PROJECT_SOURCE_DIR}/logo/debug")




# from "include(cmake/platform.cmake)"
# Detects the current platform to built to.
# Calls "include(cmake/platform_*.cmake)" depending on the platform, which:
#   - sets variables and definitions:
#     - "add_definitions(-DMIA_PLATFORM_*)"
#     - "set(MIA_PLATFORM_* true PARENT_SCOPE)"
#   - includes functions:
#     - "function(platform_options)"
#     - "function(platform_link)"
#     - "function(platform_files)"
platform_detection()


#
# Evaluate options
#

message(STATUS "[Mia] Evaluate options")

# from "include(cmake/platform_*.cmake)"
#   loaded by "function(platform_detection)" from "include(cmake/platform.cmake)"
# Evaluates platform specific options, as an example MIA_PLATFORM_CXXDROID only supports SDL2
platform_options()

# all platforms:
if (MIA_USE_SDL2)
    message(STATUS "[Mia]   MIA_USE_SDL2")
    add_definitions(-DMIA_OPTION_SDL2)
else()
    # SDL3
    if(MIA_USE_VENDOR_SDL_IMAGE OR MIA_USE_VENDOR_SDL_NET OR MIA_USE_VENDOR_SDL_TTF)
        if(NOT MIA_USE_VENDOR_SDL)
            message(STATUS "[Mia]   MIA_USE_VENDOR_SDL -> ON: vendored ports need the vendored SDL")
        endif()
    endif()
endif ()
if (MIA_USE_TTF)
    message(STATUS "[Mia]   MIA_USE_TTF")
    add_definitions(-DMIA_OPTION_TTF)
endif ()
if (MIA_USE_THREAD)
    message(STATUS "[Mia]   MIA_USE_THREAD")
    add_definitions(-DMIA_OPTION_THREAD)
endif ()
if (MIA_USE_SOCKET)
    message(STATUS "[Mia]   MIA_USE_SOCKET")
    add_definitions(-DMIA_OPTION_SOCKET)
endif ()
if (MIA_USE_FETCH)
    message(STATUS "[Mia]   MIA_USE_FETCH")
    add_definitions(-DMIA_OPTION_FETCH)
endif ()
if (MIA_USE_ZIP)
    message(STATUS "[Mia]   MIA_USE_ZIP")
    add_definitions(-DMIA_OPTION_ZIP)
endif ()
if (MIA_USE_GAMEPAD)
    if (MIA_USE_SDL2)
        message(STATUS "[Mia]   MIA_USE_GAMEPAD -> OFF: only available in SDL3")
        set(MIA_USE_GAMEPAD OFF)
    else()
        message(STATUS "[Mia]   MIA_USE_GAMEPAD")
        add_definitions(-DMIA_OPTION_GAMEPAD)
    endif()
endif ()
if (MIA_USE_TESTS)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Release" AND NOT MIA_USE_TESTS_RELEASE)
        message(STATUS "[Mia]   MIA_USE_TESTS -> OFF: Release mode, use MIA_USE_TESTS_RELEASE to activate again")
        set(MIA_USE_TESTS OFF)
    else()
        message(STATUS "[Mia]   MIA_USE_TESTS")
        add_definitions(-DMIA_OPTION_TESTS)
    endif()
endif ()
if (MIA_USE_GL_CHECK)
    message(STATUS "[Mia]   MIA_USE_GL_CHECK")
    add_definitions(-DMIA_OPTION_GL_CHECK)
endif ()
if (MIA_USE_LOG_TRACE)
    message(STATUS "[Mia]   MIA_USE_LOG_TRACE")
    add_definitions(-DMIA_OPTION_LOG_TRACE)
endif ()
if (MIA_USE_LOG_DEBUG)
    message(STATUS "[Mia]   MIA_USE_LOG_DEBUG")
    add_definitions(-DMIA_OPTION_LOG_DEBUG)
endif ()
if (MIA_USE_FILE_WARN_NATIVE)
    message(STATUS "[Mia]   MIA_USE_FILE_WARN_NATIVE")
    add_definitions(-DMIA_OPTION_FILE_WARN_NATIVE)
endif ()
if (MIA_USE_METRICS)
    message(STATUS "[Mia]   MIA_USE_METRICS")
    add_definitions(-DMIA_OPTION_METRICS)
endif ()
if (MIA_USE_METRICS_SYS)
    message(STATUS "[Mia]   MIA_USE_METRICS_SYS")
    add_definitions(-DMIA_OPTION_METRICS_SYS)
endif ()
if (MIA_USE_BREADCRUMPS)
    message(STATUS "[Mia]   MIA_USE_BREADCRUMPS")
    add_definitions(-DMIA_OPTION_BREADCRUMPS)
endif ()
if (MIA_USE_CPUGPU_SYNC)
    message(STATUS "[Mia]   MIA_USE_CPUGPU_SYNC")
    add_definitions(-DMIA_OPTION_CPUGPU_SYNC)
endif ()






#
# Import apps
#

message(STATUS "[Mia] Import apps")

set(MIA_SRCS_APP "")

# Function for app scripts to set the sources
function(mia_app_sources app_sources)
    set(MIA__APP_CURRENT_SRCS "")
    list(APPEND MIA__APP_CURRENT_SRCS ${${app_sources}})
    # Export to parent scope
    set(MIA__APP_CURRENT_SRCS "${MIA__APP_CURRENT_SRCS}" PARENT_SCOPE)

endfunction()

# Function for app scripts to call if the app should be build
function(mia_app_build)
    set(MIA__APP_BUILD true PARENT_SCOPE)
endfunction()

# Function to update MIA_LOGO
function(mia_app_logo logo_dir)
    set(MIA_LOGO "${logo_dir}" PARENT_SCOPE)
endfunction()

file(GLOB MIA__APP_SCRIPT_LIST "${CMAKE_CURRENT_LIST_DIR}/apps/*/app.cmake")
if(NOT MIA__APP_SCRIPT_LIST)
    message(WARNING "[Mia]   DID NOT FOUND ANY APP (apps/*/app.cmake)")
endif()

# tries to import every found app with its app.cmake script
foreach(MIA__APP_SCRIPT ${MIA__APP_SCRIPT_LIST})
    get_filename_component(MIA__APP_DIR ${MIA__APP_SCRIPT} DIRECTORY)
    get_filename_component(MIA__APP_NAME ${MIA__APP_DIR} NAME)
    set(MIA__APP_CURRENT_SRCS "")
    set(MIA__APP_BUILD false)
    message(STATUS "[Mia]   Importing app: ${MIA__APP_NAME}")
    include(${MIA__APP_SCRIPT})
    if(NOT MIA__APP_BUILD)
        message(STATUS "[Mia]     Aborted: ${MIA__APP_NAME}")
    else()
        message(STATUS "[Mia]     Imported: ${MIA__APP_NAME}")
        list(APPEND MIA_SRCS_APP ${MIA__APP_CURRENT_SRCS})

        # if <app>/include and <app>/res exist, use these automatically
        set(MIA__APP_INCLUDE "${MIA__APP_DIR}/include")
        set(MIA__APP_RESOURCES "${MIA__APP_DIR}/res")
        if(EXISTS ${MIA__APP_INCLUDE})
            message(STATUS "[Mia]     Adding app includes: ${MIA__APP_INCLUDE}")
            include_directories(${MIA__APP_INCLUDE})
        endif()
        if(EXISTS ${MIA__APP_RESOURCES})
            message(STATUS "[Mia]     Copying app resources: ${MIA__APP_RESOURCES}")
            file(COPY "${MIA__APP_RESOURCES}/" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mia_res)
        endif()
    endif()
endforeach()


#
# Find source files
#

message(STATUS "[Mia] Find source files")

include_directories(
        "${PROJECT_SOURCE_DIR}/mia/include"
        "${PROJECT_SOURCE_DIR}/mia/src"
)

# Glob all user source files
file(GLOB MIA_SRCS_INC "${PROJECT_SOURCE_DIR}/mia/include/*")

# Unitybuild
if (MIA_USE_UNITYBUILD)
    message(STATUS "[Mia]   Unity build")
    add_definitions(-DMIA_OPTION_UNITYBUILD)

    # select only the unity build file
    set(MIA_SRCS_O "${PROJECT_SOURCE_DIR}/mia/src/o/o.c")
    set(MIA_SRCS_M "${PROJECT_SOURCE_DIR}/mia/src/m/m.c")
    set(MIA_SRCS_S "${PROJECT_SOURCE_DIR}/mia/src/s/s.c")
    set(MIA_SRCS_R "${PROJECT_SOURCE_DIR}/mia/src/r/r.c")
    set(MIA_SRCS_A "${PROJECT_SOURCE_DIR}/mia/src/a/a.c")
    set(MIA_SRCS_U "${PROJECT_SOURCE_DIR}/mia/src/u/u.c")
    set(MIA_SRCS_W "${PROJECT_SOURCE_DIR}/mia/src/w/w.c")
    set(MIA_SRCS_X "${PROJECT_SOURCE_DIR}/mia/src/x/x.c")

    if (MIA_USE_TESTS)
        set(MIA_SRCS_TESTS  "${PROJECT_SOURCE_DIR}/mia/test/test.c")
    endif ()
else ()

    # glob all source files
    file(GLOB MIA_SRCS_O "${PROJECT_SOURCE_DIR}/mia/src/o/*.c")
    file(GLOB MIA_SRCS_M "${PROJECT_SOURCE_DIR}/mia/src/m/*.c")
    file(GLOB MIA_SRCS_S "${PROJECT_SOURCE_DIR}/mia/src/s/*.c")
    file(GLOB MIA_SRCS_R "${PROJECT_SOURCE_DIR}/mia/src/r/*.c")
    file(GLOB MIA_SRCS_A "${PROJECT_SOURCE_DIR}/mia/src/a/*.c")
    file(GLOB MIA_SRCS_U "${PROJECT_SOURCE_DIR}/mia/src/u/*.c")
    file(GLOB MIA_SRCS_W "${PROJECT_SOURCE_DIR}/mia/src/w/*.c")
    file(GLOB MIA_SRCS_X "${PROJECT_SOURCE_DIR}/mia/src/x/*.c")

    # remove unitybuild source files
    list(REMOVE_ITEM MIA_SRCS_O "${PROJECT_SOURCE_DIR}/mia/src/o/o.c")
    list(REMOVE_ITEM MIA_SRCS_M "${PROJECT_SOURCE_DIR}/mia/src/m/m.c")
    list(REMOVE_ITEM MIA_SRCS_S "${PROJECT_SOURCE_DIR}/mia/src/s/s.c")
    list(REMOVE_ITEM MIA_SRCS_R "${PROJECT_SOURCE_DIR}/mia/src/r/r.c")
    list(REMOVE_ITEM MIA_SRCS_A "${PROJECT_SOURCE_DIR}/mia/src/a/a.c")
    list(REMOVE_ITEM MIA_SRCS_U "${PROJECT_SOURCE_DIR}/mia/src/u/u.c")
    list(REMOVE_ITEM MIA_SRCS_W "${PROJECT_SOURCE_DIR}/mia/src/w/w.c")
    list(REMOVE_ITEM MIA_SRCS_X "${PROJECT_SOURCE_DIR}/mia/src/x/x.c")

    if (MIA_USE_TESTS)
        file(GLOB MIA_SRCS_TESTS
                "${PROJECT_SOURCE_DIR}/mia/test/*"
                "${PROJECT_SOURCE_DIR}/mia/test/o/*"
                "${PROJECT_SOURCE_DIR}/mia/test/m/*"
                "${PROJECT_SOURCE_DIR}/mia/test/r/*"
        )
        list(REMOVE_ITEM MIA_SRCS_TESTS "${PROJECT_SOURCE_DIR}/mia/test/test.c")
    endif ()
endif ()



# Combine all source lists
set(MIA_SRCS "")
list(APPEND MIA_SRCS
#        ${MIA_SRCS_INC}
        ${MIA_SRCS_O}
        ${MIA_SRCS_M}
        ${MIA_SRCS_S}
        ${MIA_SRCS_R}
        ${MIA_SRCS_A}
        ${MIA_SRCS_U}
        ${MIA_SRCS_W}
        ${MIA_SRCS_X}
        ${MIA_SRCS_APP}
)


if(MIA_USE_BUILT_AS_LIBRARY)
    message(STATUS "[Mia] Defining library")
    add_library(${MIA_PROJECT} SHARED ${MIA_SRCS} ${MIA_SRCS_TESTS})
else ()
    message(STATUS "[Mia] Defining executable")
    add_executable(${MIA_PROJECT} ${MIA_SRCS} ${MIA_SRCS_TESTS})
endif()



#
# Linking
#

message(STATUS "[Mia] Linking")

# from "include(cmake/platform_*.cmake)"
#   loaded by "function(platform_detection)" from "include(cmake/platform.cmake)"
# Links external libraries like SDL depending on the platform.
platform_link()



#
# Cloning resources
#

message(STATUS "[Mia] Cloning resources")
message(STATUS "[Mia]   MIA_LOGO: ${MIA_LOGO}")


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mia/res/
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mia_res)

# from "include(cmake/platform_*.cmake)"
#   loaded by "function(platform_detection)" from "include(cmake/platform.cmake)"
# Copies resources, etc. depending on the platform.
platform_files()


message(STATUS "[Mia] Done ;)")

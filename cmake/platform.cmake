
#
# Detects the current platform and includes its cmake scripts.
# Needs to be run after the project(...) definition
#

function(platform_detection)
    message(STATUS "[Mia] Platform detection")

    if (EMSCRIPTEN)
        include(cmake/platform_emscripten.cmake)
    elseif (MINGW)
        include(cmake/platform_mingw.cmake)
    elseif (WIN32)
        include(cmake/platform_msvc.cmake)
    elseif (APPLE)
        include(cmake/platform_macos.cmake)
    elseif (CMAKE_COMMAND MATCHES "cxxdroid")
        # *.cxxdroid.* is part of the cmake path in cxxdroid
        include(cmake/platform_cxxdroid.cmake)
	elseif (ANDROID)
	    include(cmake/platform_android.cmake)
    elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        include(cmake/platform_linux.cmake)
    else ()
        message(FATAL_ERROR "[Mia]   Platform: unknown!")
    endif ()
endfunction()

#
# Sets the (vcpkg) toolchain automatically, if present.
# Needs to be run before project(...) definition
#

function(platform_set_toolchain)
    if(WIN32)
        set(VCPKG_PRIMARY_PATH "C:/dev/vcpkg")
        set(VCPKG_FALLBACK_PATH "D:/dev/vcpkg")
        if (EXISTS ${VCPKG_PRIMARY_PATH})
            set(CMAKE_TOOLCHAIN_FILE "${VCPKG_PRIMARY_PATH}/scripts/buildsystems/vcpkg.cmake" PARENT_SCOPE)
            message(STATUS "[Mia]     Platform: Windows MSVC: Using vcpkg from primary path: ${VCPKG_PRIMARY_PATH}")
        elseif (EXISTS ${VCPKG_FALLBACK_PATH})
            set(CMAKE_TOOLCHAIN_FILE "${VCPKG_FALLBACK_PATH}/scripts/buildsystems/vcpkg.cmake" PARENT_SCOPE)
            message(STATUS "[Mia]     Using vcpkg from fallback path: ${VCPKG_FALLBACK_PATH}")
        else()
            message(WARNING "[Mia]    vcpkg not found in either primary (${VCPKG_PRIMARY_PATH}) or fallback (${VCPKG_FALLBACK_PATH}) locations. Use -DCMAKE_TOOLCHAIN_FILE=.../vcpkg.cmake to set it")
        endif()
    endif()
endfunction()

#
# function that links sdl3 xPlatform, may be used in the various platform_* cmake scripts
#

function(platform_link_sdl3)

    # so the vendored stuff is compiled into build directly
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "vendor to build dir")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "vendor to build dir")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}" CACHE PATH "vendor to build dir")

    if(MIA_USE_VENDOR_SDL)
        add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)
    else()
        find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
    endif()
    # linked at the end of the function

    if(MIA_USE_VENDOR_SDL_IMAGE)
        # disable formats we don't use to make the build faster and smaller.
        set(SDLIMAGE_VENDORED ON CACHE INTERNAL "Enable vendor mode for SDL_image")
        set(SDLIMAGE_AVIF OFF CACHE INTERNAL "Disable AVIF support for SDL_image")
        set(SDLIMAGE_AVIF_SAVE OFF CACHE INTERNAL "Disable AVIF_SAVE support for SDL_image")
        set(SDLIMAGE_AVIF_SHARED OFF CACHE INTERNAL "Disable AVIF_SHARED support for SDL_image")
        set(SDLIMAGE_LBM OFF CACHE INTERNAL "Disable LBM support for SDL_image")
        set(SDLIMAGE_PCX OFF CACHE INTERNAL "Disable PCX support for SDL_image")
        set(SDLIMAGE_PNM OFF CACHE INTERNAL "Disable PNM support for SDL_image")
        set(SDLIMAGE_QOI OFF CACHE INTERNAL "Disable QOI support for SDL_image")
        set(SDLIMAGE_SVG OFF CACHE INTERNAL "Disable SVG support for SDL_image")
        set(SDLIMAGE_TGA OFF CACHE INTERNAL "Disable TGA support for SDL_image")
        set(SDLIMAGE_TIF OFF CACHE INTERNAL "Disable TIF support for SDL_image")
        set(SDLIMAGE_TIF_SHARED OFF CACHE INTERNAL "Disable TIF_SHARED support for SDL_image")
        set(SDLIMAGE_WEBP OFF CACHE INTERNAL "Disable WEBP support for SDL_image")
        set(SDLIMAGE_WEBP_SHARED OFF CACHE INTERNAL "Disable WEBP_SHARED support for SDL_image")
        set(SDLIMAGE_XCF OFF CACHE INTERNAL "Disable XCF support for SDL_image")
        set(SDLIMAGE_XPM OFF CACHE INTERNAL "Disable XPM support for SDL_image")
        set(SDLIMAGE_XV OFF CACHE INTERNAL "Disable XV support for SDL_image")

        add_subdirectory(vendor/SDL_image EXCLUDE_FROM_ALL)
    else()
        find_package(SDL3_image REQUIRED CONFIG)
    endif()
    target_link_libraries(${MIA_PROJECT} SDL3_image::SDL3_image)

    if (MIA_USE_SOCKET)
        if(MIA_USE_VENDOR_SDL_NET)
            add_subdirectory(vendor/SDL_net EXCLUDE_FROM_ALL)
        else()
            find_package(SDL3_net REQUIRED CONFIG)
        endif()
        target_link_libraries(${MIA_PROJECT} SDL3_net::SDL3_net)
    endif ()

    if (MIA_USE_TTF)
        if(MIA_USE_VENDOR_SDL_TTF)
            set(SDLTTF_VENDORED ON CACHE INTERNAL "Enable vendor mode for SDL_ttf")
            add_subdirectory(vendor/SDL_ttf EXCLUDE_FROM_ALL)
        else()
            find_package(SDL3_ttf REQUIRED CONFIG)
        endif()
        target_link_libraries(${MIA_PROJECT} SDL3_ttf::SDL3_ttf)
    endif ()

    # needs to be last linked
    target_link_libraries(${MIA_PROJECT} SDL3::SDL3)

endfunction()

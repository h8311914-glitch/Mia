message(STATUS "[Mia]   Platform: Windows MSVC")
add_definitions(-DMIA_PLATFORM_MSVC)
add_definitions(-DMIA_PLATFORM_WINDOWS)
add_definitions(-DMIA_PLATFORM_DESKTOP)
set(MIA_PLATFORM_MSVC true PARENT_SCOPE)
set(MIA_PLATFORM_WINDOWS true PARENT_SCOPE)
set(MIA_PLATFORM_DESKTOP true PARENT_SCOPE)

#
# Evaluate options, called in CMakeLists.txt
#

function(platform_options)
    if (NOT MIA_USE_THREAD AND MIA_USE_FETCH)
        message(STATUS "[Mia]   MIA_USE_FETCH -> OFF: MIA_PLATFORM_MSVC needs threads for fetching")
        set(MIA_USE_FETCH false PARENT_SCOPE)
    endif()
endfunction()


#
# Link external libraries like SDL, called in CMakeLists.txt
#

function(platform_link)
    message(STATUS "[Mia]   Setup Windows MSVC")

    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /WX")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267 /wd4244 /wd4305 /wd4996 /wd5105" PARENT_SCOPE)

    find_package(GLEW REQUIRED)
    add_definitions(-DMIA_OPTION_GLEW)

    target_link_libraries(${MIA_PROJECT}
            GLEW::GLEW
            comdlg32        # e_ext_sfd
    )

    if (MIA_USE_SDL2)
        add_definitions(-Dmain=SDL_main)

        find_package(SDL2 REQUIRED)
        find_package(SDL2_image CONFIG REQUIRED)

        target_link_libraries(${MIA_PROJECT}
                SDL2::SDL2
                SDL2::SDL2main
                SDL2_image::SDL2_image
        )

        if (MIA_USE_SOCKET)
            find_package(SDL2_net CONFIG REQUIRED)
            target_link_libraries(${MIA_PROJECT} SDL2_net::SDL2_net ws2_32)
        endif ()
        if (MIA_USE_TTF)
            find_package(SDL2_ttf CONFIG REQUIRED)
            target_link_libraries(${MIA_PROJECT} SDL2_ttf::SDL2_ttf)
        endif ()

    else ()
        # SDL3
        platform_link_sdl3()
    endif ()

    if (MIA_USE_FETCH)
        find_package(CURL CONFIG REQUIRED)
        target_link_libraries(${MIA_PROJECT} CURL::libcurl)
    endif ()
endfunction()

#
# Copy files like resources, called in CMakeLists.txt
#

function(platform_files)
    # also copy res to MSVC subdirs Debug and Release
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/mia_res/
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/mia_res)
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/mia_res/
            DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/mia_res)

endfunction()

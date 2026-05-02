message(STATUS "[Mia]   Platform: Windows MinGW")
message(WARNING "[Mia]     MinGW is currently experimental")
add_definitions(-DMIA_PLATFORM_MINGW)
add_definitions(-DMIA_PLATFORM_WINDOWS)
add_definitions(-DMIA_PLATFORM_DESKTOP)
set(MIA_PLATFORM_MINGW true PARENT_SCOPE)
set(MIA_PLATFORM_WINDOWS true PARENT_SCOPE)
set(MIA_PLATFORM_DESKTOP true PARENT_SCOPE)

#
# Evaluate options, called in CMakeLists.txt
#

function(platform_options)
    if (NOT MIA_USE_THREAD AND MIA_USE_FETCH)
        message(STATUS "[Mia]   MIA_USE_FETCH -> OFF: MIA_PLATFORM_MINGW needs threads for fetching")
        set(MIA_USE_FETCH false PARENT_SCOPE)
    endif()
endfunction()


#
# Link external libraries like SDL, called in CMakeLists.txt
#

function(platform_link)
    message(STATUS "[Mia]   Setup Windows MinGW")
    message(STATUS "[Mia]     currently experimental")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -Wno-unused-parameter -Wno-format-truncation" PARENT_SCOPE)

    add_definitions(-DMIA_OPTION_GLEW)

    # mingw may not find a few libraries, due to missing CMAKE_LIBRARY_PATH lib paths
    find_library(OPENGL_LIB opengl32 REQUIRED)
    find_library(GLEW_LIB glew32 REQUIRED)
    find_library(GLU_LIB glu32 REQUIRED)

    target_link_libraries(${MIA_PROJECT}
            m
            mingw32
            comdlg32        # e_ext_sfd
            ${OPENGL_LIB}
            ${GLEW_LIB}
            ${GLU_LIB}
    )

    # mingw is not shipped with a find_package(SDL2) script
    if (MIA_USE_SDL2)
        add_definitions(-Dmain=SDL_main)

        target_link_libraries(${MIA_PROJECT}
                SDL2main
                SDL2
                SDL2_image
        )
        if (MIA_USE_SOCKET)
            target_link_libraries(${MIA_PROJECT} SDL2_net ws2_32)
        endif ()
        if (MIA_USE_TTF)
            target_link_libraries(${MIA_PROJECT} SDL2_ttf)
        endif ()

    else()
        # SDL3
        platform_link_sdl3()
    endif()

    if (MIA_USE_FETCH)
        target_link_libraries(${MIA_PROJECT} curl)
    endif ()
endfunction()

#
# Copy files like resources, called in CMakeLists.txt
#

function(platform_files)
    # noop
endfunction()

message(STATUS "[Mia]   Platform: MacOs")
add_definitions(-DMIA_PLATFORM_MACOS)
add_definitions(-DMIA_PLATFORM_DESKTOP)
set(MIA_PLATFORM_MACOS true PARENT_SCOPE)
set(MIA_PLATFORM_DESKTOP true PARENT_SCOPE)

#
# Evaluate options, called in CMakeLists.txt
#

function(platform_options)
    if (NOT MIA_USE_THREAD AND MIA_USE_FETCH)
        message(STATUS "[Mia]   MIA_USE_FETCH -> OFF: MIA_PLATFORM_MACOS needs threads for fetching")
        set(MIA_USE_FETCH false PARENT_SCOPE)
    endif()
endfunction()


#
# Link external libraries like SDL, called in CMakeLists.txt
#

function(platform_link)
    message(STATUS "[Mia]   Setup MacOs")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -Wno-unused-parameter" PARENT_SCOPE)

    if (MIA_USE_SANITIZER)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "[Mia]     Using Google sanitizer")
            add_definitions(-DMIA_OPTION_SANITIZER)
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" PARENT_SCOPE)
            set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" PARENT_SCOPE)
        endif ()
    endif ()

    find_package(OpenGL REQUIRED)

    target_link_libraries(${MIA_PROJECT}
            m
            ${OPENGL_LIBRARIES}
    )

    if (MIA_USE_SDL2)
        find_package(SDL2 REQUIRED)
        find_library(SDL2_IMAGE_LIBRARIES SDL2_image REQUIRED)

        # somehow this is not included by default and will not get set (even if using modern find_package stuff)
        target_include_directories(${MIA_PROJECT} PRIVATE /opt/homebrew/include)

        target_link_libraries(${MIA_PROJECT}
                ${SDL2_LIBRARIES}
                ${SDL2_IMAGE_LIBRARIES}
        )

        if (MIA_USE_SOCKET)
            find_library(SDL2_NET_LIBRARIES SDL2_net REQUIRED)
            target_link_libraries(${MIA_PROJECT} ${SDL2_NET_LIBRARIES})
        endif ()
        if (MIA_USE_TTF)
            find_library(SDL2_TTF_LIBRARIES SDL2_ttf REQUIRED)
            target_link_libraries(${MIA_PROJECT} ${SDL2_TTF_LIBRARIES})
        endif ()

    else ()
        # SDL3
        platform_link_sdl3()
    endif ()

    # not necessary in macos
    find_library(GLEW_LIB GLEW)
    if (GLEW_LIB)
        add_definitions(-DMIA_OPTION_GLEW)
        target_link_libraries(${MIA_PROJECT} ${GLEW_LIB})
    endif ()

    if (MIA_USE_FETCH)
        find_package(CURL REQUIRED)
        if (NOT CURL_LIBRARIES)
            # stupid cmake bug...
            message(STATUS "[Mia]     MIA_USE_FETCH -> Failed to collect \"CURL_LIBRARIES\", setting it to \"/usr/lib/x86_64-linux-gnu/libcurl.so\"")
            set(CURL_LIBRARIES "/usr/lib/x86_64-linux-gnu/libcurl.so" PARENT_SCOPE)
        endif ()
        target_link_libraries(${MIA_PROJECT} ${CURL_LIBRARIES})
    endif ()
endfunction()

#
# Copy files like resources, called in CMakeLists.txt
#

function(platform_files)
    # noop
endfunction()

message(STATUS "[Mia]   Platform: Linux")
add_definitions(-DMIA_PLATFORM_LINUX)
add_definitions(-DMIA_PLATFORM_DESKTOP)
set(MIA_PLATFORM_LINUX true PARENT_SCOPE)
set(MIA_PLATFORM_DESKTOP true PARENT_SCOPE)

#
# Evaluate options, called in CMakeLists.txt
#

function(platform_options)
    if (MIA_USE_TTF AND MIA_USE_SANITIZER)
        message(WARNING "[Mia]   MIA_USE_TTF + MIA_USE_SANITIZER may not work under linux")
    endif ()
    if (NOT MIA_USE_THREAD AND MIA_USE_FETCH)
        message(STATUS "[Mia]   MIA_USE_FETCH -> OFF: MIA_PLATFORM_LINUX needs threads for fetching")
        set(MIA_USE_FETCH false PARENT_SCOPE)
    endif()
endfunction()


#
# Link external libraries like SDL, called in CMakeLists.txt
#

function(platform_link)
    message(STATUS "[Mia]   Setup Linux")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -Wno-unused-parameter]" PARENT_SCOPE)

    if (MIA_USE_SANITIZER)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "[Mia]     Using Google sanitizer")
            add_definitions(-DMIA_OPTION_SANITIZER)
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" PARENT_SCOPE)
            set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" PARENT_SCOPE)
        endif ()
    endif ()

    find_library(GL_LIB GL REQUIRED)

    target_link_libraries(${MIA_PROJECT}
            m
            ${GL_LIB}
    )

    if (MIA_USE_SDL2)
        find_package(SDL2 REQUIRED)
        find_library(SDL2_IMAGE_LIBRARIES SDL2_image REQUIRED)

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

    # not necessary in linux
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

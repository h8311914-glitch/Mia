message(STATUS "[Mia]   Platform: CxxDroid")
add_definitions(-DMIA_PLATFORM_CXXDROID)
set(MIA_PLATFORM_CXXDROID true PARENT_SCOPE)

#
# Evaluate options, called in CMakeLists.txt
#

function(platform_options)
    if (NOT MIA_USE_SDL2)
        message(STATUS "[Mia]   MIA_USE_SDL2 -> ON: MIA_PLATFORM_CXXDROID does not support SDL3 yet")
        set(MIA_USE_SDL2 true PARENT_SCOPE)
    endif ()
    if (NOT MIA_USE_THREAD AND MIA_USE_FETCH)
        message(STATUS "[Mia]   MIA_USE_FETCH -> OFF: MIA_PLATFORM_CXXDROID needs threads for fetching")
        set(MIA_USE_FETCH false PARENT_SCOPE)
    endif()


    # some options just to set em here, cause CxxDroid does not allow nice cmake cache setups.
    # (or just to test faster...)
    # so these change alot...
    
    set(MIA_USE_APP_HELLO true PARENT_SCOPE)
    
    set(MIA_USE_APP_MP true PARENT_SCOPE)
    set(MIA_USE_APP_MP_MAIN false PARENT_SCOPE)
    set(MIA_USE_APP_EX true PARENT_SCOPE)
    set(MIA_USE_APP_EX_MAIN false PARENT_SCOPE)
    set(MIA_USE_APP_EX_MAIN_TEA false PARENT_SCOPE)
    set(MIA_USE_SOCKET true PARENT_SCOPE)
    set(MIA_USE_BREADCRUMPS true PARENT_SCOPE)
    # set(MIA_USE_UNITYBUILD false PARENT_SCOPE)
    # set(CMAKE_BUILD_TYPE "Release" PARENT_SCOPE)

endfunction()


#
# Link external libraries like SDL, called in CMakeLists.txt
#

function(platform_link)
    message(STATUS "[Mia]   Setup CxxDroid")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -Wno-unused-parameter" PARENT_SCOPE)

    add_definitions(-DMIA_OPTION_GLES)
    add_definitions(-DMIA_OPTION_TERMINALCOLOR_OFF)
    add_definitions(-DMIA_OPTION_LOG_COMPACT)

    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)

    find_library(GLES_LIB GLESv3 REQUIRED)

    target_link_libraries(${MIA_PROJECT}
            m
            ${SDL2_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            ${GLES_LIB}
    )

    if (MIA_USE_SOCKET)
        find_package(SDL2_net REQUIRED)
        target_link_libraries(${MIA_PROJECT} ${SDL2_NET_LIBRARIES})
    endif ()
    if (MIA_USE_TTF)
        find_package(SDL2_ttf REQUIRED)
        target_link_libraries(${MIA_PROJECT} ${SDL2_TTF_LIBRARIES})
    endif ()
    if (MIA_USE_FETCH)
        find_package(CURL REQUIRED)
        target_link_libraries(${MIA_PROJECT} ${CURL_LIBRARIES})
    endif ()
endfunction()

#
# Copy files like resources, called in CMakeLists.txt
#

function(platform_files)
    # noop
endfunction()
if ()
endfunction()

#
# Copy files like resources, called in CMakeLists.txt
#

function(platform_files)
    # noop
endfunction()

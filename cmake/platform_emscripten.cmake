message(STATUS "[Mia]   Platform: Emscripten")
add_definitions(-DMIA_PLATFORM_EMSCRIPTEN)
set(MIA_PLATFORM_EMSCRIPTEN true PARENT_SCOPE)

#
# Evaluate options, called in CMakeLists.txt
#

function(platform_options)
    if (MIA_USE_THREAD AND NOT MIA_USE_THREAD_EMSCRIPTEN)
        message(STATUS "[Mia]   MIA_USE_THREAD -> OFF: Enable MIA_USE_THREAD_EMSCRIPTEN to use threads in emscripten")
        set(MIA_USE_THREAD false PARENT_SCOPE)
    endif ()
    if (MIA_USE_SOCKET)
        message(STATUS "[Mia]   MIA_USE_SOCKET -> OFF: MIA_PLATFORM_EMSCRIPTEN does not support sockets by default")
        set(MIA_USE_SOCKET false PARENT_SCOPE)
    endif ()
    if (MIA_USE_SANITIZER)
        message(STATUS "[Mia]   MIA_USE_SANITIZER -> OFF: Emscripten is able to do it, but may crash the app")
        set(MIA_USE_SANITIZER false PARENT_SCOPE)
    endif ()
    if(NOT MIA_USE_SDL2 AND NOT MIA_USE_VENDOR_SDL)
        message(STATUS "[Mia]   MIA_USE_VENDOR_SDL -> ON: Emscripten is not bundled with SDL3 yet")
        set(MIA_USE_VENDOR_SDL true PARENT_SCOPE)
    endif()
    if(NOT MIA_USE_SDL2 AND NOT MIA_USE_VENDOR_SDL_IMAGE)
        message(STATUS "[Mia]   MIA_USE_VENDOR_SDL_IMAGE -> ON: Emscripten is not bundled with SDL3_image yet")
        set(MIA_USE_VENDOR_SDL_IMAGE true PARENT_SCOPE)
    endif()
    if(NOT MIA_USE_SDL2 AND MIA_USE_TTF AND NOT MIA_USE_VENDOR_SDL_TTF)
        message(STATUS "[Mia]   MIA_USE_VENDOR_SDL_TTF -> ON: Emscripten is not bundled with SDL3_ttf yet")
        set(MIA_USE_VENDOR_SDL_TTF true PARENT_SCOPE)
    endif()
endfunction()


#
# Link external libraries like SDL, called in CMakeLists.txt
#

function(platform_link)
    message(STATUS "[Mia]   Setup Emscripten")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments" PARENT_SCOPE)

    if (MIA_USE_SANITIZER)
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(STATUS "[Mia]     Using Google sanitizer")
            add_definitions(-DMIA_OPTION_SANITIZER)
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" PARENT_SCOPE)
            set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address -fsanitize=undefined" PARENT_SCOPE)
        endif ()
    endif ()

    add_definitions(-DMIA_OPTION_GLES)
    add_definitions(-DMIA_OPTION_TERMINALCOLOR_OFF)
    add_definitions(-DMIA_OPTION_LOG_COMPACT)

    # system libs
    if (MIA_USE_SDL2)
        set(EMS_LIBS "-sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='[\"png\", \"jpg\"]'")
        if (MIA_USE_TTF)
            set(EMS_LIBS "${EMS_LIBS} -sUSE_SDL_TTF=2")
        endif ()
    else()

        # sdl3 vendored see below

        # system libs not available yet
        #        set(EMS_LIBS "-sUSE_SDL=3 -sUSE_SDL_IMAGE=3 -sSDL3_IMAGE_FORMATS='[\"png\", \"jpg\"]'")
        #        if (MIA_USE_TTF)
        #            set(EMS_LIBS "${EMS_LIBS} -sUSE_SDL_TTF=3")
        #        endif ()
    endif()

    if (MIA_USE_THREAD)
        message(STATUS "[Mia]     Compiling with pthreads, fixed memory size")
        message(STATUS "[Mia]     Needs a server with COOP & COEP headers set correctly, have a look at web/thread_server.py")
        set(EMS_LIBS "${EMS_LIBS} -pthread -sUSE_PTHREADS")
        set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sPTHREAD_POOL_SIZE=8")
        set(SDL_PTHREADS ON PARENT_SCOPE)
    endif()


    # compile step needs the libs flags, so emscripten ports are automatically installed
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMS_LIBS}" PARENT_SCOPE)

    # link flags
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} ${EMS_LIBS}")
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sFULL_ES3=1")
    if (MIA_USE_FETCH)
        set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sFETCH=1")
    endif ()

    # default stack size is only 64K, using 256K instead
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sSTACK_SIZE=250880")
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sEXPORTED_FUNCTIONS='[\"_main\", \"_o_file_route_save__idbfs_synced\", \"_o_file_upload__done\"]'")
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sEXPORTED_RUNTIME_METHODS=FS,ccall")
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sEXIT_RUNTIME")
    
    # note: DO NOT USE ASYNCIFY
    #       IT ENLARGES THE WASM CODE TOO MUCH
    #       BREAKS ON IOS SAFARI
    
    if (NOT MIA_USE_THREAD)
        set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -sALLOW_MEMORY_GROWTH")
    endif()
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} -lidbfs.js")
    set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} --preload-file mia_res")
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "[Mia]     Compiling with support for emrun")
        set(EMS_LINK_FLAGS "${EMS_LINK_FLAGS} --emrun -Wno-limited-postlink-optimizations")
    endif ()

    set_target_properties(${MIA_PROJECT} PROPERTIES LINK_FLAGS ${EMS_LINK_FLAGS})

    # link vendored after setting link flags, so the vendored versions get compiled correctly
    if(NOT MIA_USE_SDL2)
        platform_link_sdl3()
    endif()


    set_target_properties(${MIA_PROJECT} PROPERTIES OUTPUT_NAME "index" SUFFIX ".js")
endfunction()

#
# Copy files like resources, called in CMakeLists.txt
#

function(platform_files)
    # Custom index.html
    configure_file(${CMAKE_SOURCE_DIR}/web/index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)

    # icons used by the web/index.html
    # remove first for "forced copy"

    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/favicon.ico)
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/icon180.png)
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/icon196.ico)
    file(COPY ${MIA_LOGO}/favicon.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${MIA_LOGO}/icon180.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY ${MIA_LOGO}/icon196.png DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    # debug website
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/web/debug.html DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endfunction()

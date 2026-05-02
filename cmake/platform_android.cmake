message(STATUS "[Mia]   Platform: Android")
add_definitions(-DMIA_PLATFORM_ANDROID)
set(MIA_PLATFORM_ANDROID true PARENT_SCOPE)

#
# Evaluate options, called in CMakeLists.txt
#

function(platform_options)
    if (NOT MIA_USE_THREAD AND MIA_USE_FETCH)
        message(STATUS "[Mia]   MIA_USE_FETCH -> OFF: MIA_PLATFORM_ANDROID needs threads for fetching")
        set(MIA_USE_FETCH false PARENT_SCOPE)
    endif()

endfunction()


#
# Link external libraries like SDL, called in CMakeLists.txt
#

function(platform_link)
    message(STATUS "[Mia]   Setup Android")

    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic" PARENT_SCOPE)
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-long-long -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -Wno-unused-parameter" PARENT_SCOPE)

    add_definitions(-DMIA_OPTION_GLES)
	add_definitions(-DMIA_INIT_FULLSCREEN)
	add_definitions(-DMIA_OPTION_LOG_VIA_SDL)
	add_definitions(-DMIA_OPTION_TERMINALCOLOR_OFF)
	add_definitions(-DMIA_OPTION_LOG_COMPACT)

    platform_link_sdl3()
	
	target_link_libraries(${MIA_PROJECT}
	        EGL
			GLESv3
	)
	
endfunction()

#
# Copy files like resources, called in CMakeLists.txt
#

function(platform_files)
    # copy res to android assets.
    # remove first for "forced copy"
    # CMAKE_CURRENT_SOURCE_DIR -> "app/jni/Mia"
    # so "CMAKE_CURRENT_SOURCE_DIR}/../.." is "app"
    # and we want it in "app/src/main/assets/mia_res/..."

    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/assets/mia_res)
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/mia_res/
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/assets/mia_res)

    # copy logos:

    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-mdpi)
    file(COPY ${MIA_LOGO}/mipmap-mdpi/
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-mdpi)

    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-hdpi)
    file(COPY ${MIA_LOGO}/mipmap-hdpi/
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-hdpi)

    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-xhdpi)
    file(COPY ${MIA_LOGO}/mipmap-xhdpi/
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-xhdpi)

    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-xxhdpi)
    file(COPY ${MIA_LOGO}/mipmap-xxhdpi/
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-xxhdpi)

    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-xxxhdpi)
    file(COPY ${MIA_LOGO}/mipmap-xxxhdpi/
            DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main/res/mipmap-xxxhdpi)

endfunction()


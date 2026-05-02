#
# App "mp" Mia Paint cmake script
#


option(MIA_USE_APP_MP "Build Mia Paint app" ON)
option(MIA_USE_APP_MP_MAIN "Includes a main fn in the Mia Paint app" OFF)


if(MIA_USE_APP_MP_MAIN)
    if(NOT MIA_USE_APP_MP)
        message(STATUS "[Mia]   MIA_USE_APP_MP_MAIN -> OFF: Needs MIA_USE_APP_MP")
        set(MIA_USE_APP_MP_MAIN false)
    else()
        message(STATUS "[Mia]   MIA_USE_APP_MP_MAIN")
        add_definitions(-DMIA_OPTION_APP_MP_MAIN)
    endif()
endif()


# only build the app if the option is set
if(NOT MIA_USE_APP_MP)
    return()
endif()

message(STATUS "[Mia App] mp")

add_definitions(-DMIA_OPTION_APP_MP)

# Search source files:
# The Mia Paint app has a unitybuild mode
if (MIA_USE_UNITYBUILD)
    set(APP_MP_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/mp.c")
else ()
    file(GLOB APP_MP_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/*.c")
    list(REMOVE_ITEM APP_MP_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/mp.c")
endif()

# Pass the source files list to mia
mia_app_sources(APP_MP_SRCS)


# /include dir is (if exists) added to the include_directories
# /res dir is (if exists) copied into the resources

# Let mia build this app
mia_app_build()


#
# App "ms" Mia Studio cmake script
#


option(MIA_USE_APP_MS "Build Mia Studio app" ON)
option(MIA_USE_APP_MS_MAIN "Includes a main fn in the Mia Studio app" OFF)

if(MIA_USE_APP_MS)
    if(NOT MIA_USE_APP_MP)
        message(STATUS "[Mia]   MIA_USE_APP_MS -> OFF: Needs MIA_USE_APP_MP")
        set(MIA_USE_APP_MS false)
    endif()
endif()

if(MIA_USE_APP_MS_MAIN)
    if(NOT MIA_USE_APP_MS)
        message(STATUS "[Mia]   MIA_USE_APP_MS_MAIN -> OFF: Needs MIA_USE_APP_MS")
        set(MIA_USE_APP_MS_MAIN false)
    else()
        message(STATUS "[Mia]   MIA_USE_APP_MS_MAIN")
        add_definitions(-DMIA_OPTION_APP_MS_MAIN)
    endif()
endif()


if(NOT MIA_USE_FETCH)
    message(STATUS "[Mia]   MIA_USE_APP_MS -> OFF: needs MIA_USE_FETCH")
    set(MIA_USE_APP_MS false)
    return()
endif()
if(NOT MIA_USE_ZIP)
    message(STATUS "[Mia]   MIA_USE_APP_MS -> OFF: needs MIA_USE_ZIP")
    set(MIA_USE_APP_MS false)
    return()
endif()


# only build the app if the option is set
if(NOT MIA_USE_APP_MS)
    return()
endif()

message(STATUS "[Mia App] ms")

# Search source files:
# The Mia Studio app has a unitybuild mode
if (MIA_USE_UNITYBUILD)
    set(APP_MS_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/ms/ms.c")
else ()
    file(GLOB APP_MS_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/ms/*.c")
    list(REMOVE_ITEM APP_MS_SRCS "${CMAKE_CURRENT_LIST_DIR}/ms/src/st.c")
endif()

# Pass the source files list to mia
mia_app_sources(APP_MS_SRCS)


# /include dir is (if exists) added to the include_directories
# /res dir is (if exists) copied into the resources

# Let mia build this app
mia_app_build()


#
# App "ex"amples cmake script
#

option(MIA_USE_APP_EX "Build examples app" ON)
option(MIA_USE_APP_EX_MAIN "Includes a main fn in the examples app" OFF)
option(MIA_USE_APP_EX_MAIN_TEA "Starts the tea app and sets the tea logo" OFF)

if(MIA_USE_APP_EX_MAIN)
    if(NOT MIA_USE_APP_EX)
        message(STATUS "[Mia]   MIA_USE_APP_EX_MAIN -> OFF: Needs MIA_USE_APP_EX")
        set(MIA_USE_APP_EX_MAIN false)
    else()
        message(STATUS "[Mia]   MIA_USE_APP_EX_MAIN")
        add_definitions(-DMIA_OPTION_APP_EX_MAIN)
    endif()
endif()
if(MIA_USE_APP_EX_MAIN_TEA)
    if(NOT MIA_USE_APP_EX_MAIN)
        message(STATUS "[Mia]   MIA_USE_APP_EX_MAIN_TEA -> OFF: Needs MIA_USE_APP_EX_MAIN")
        set(MIA_USE_APP_EX_MAIN_TEA false)
    else()
        message(STATUS "[Mia]   MIA_USE_APP_EX_MAIN_TEA")
        add_definitions(-DMIA_OPTION_APP_EX_MAIN_TEA)
    endif()
endif()


# only build the app if the option is set
if(NOT MIA_USE_APP_EX)
    return()
endif()


if(NOT MIA_USE_FETCH)
    message(STATUS "[Mia]   MIA_USE_APP_EX -> OFF: Examples app needs MIA_USE_FETCH")
    set(MIA_USE_APP_EX false)
    return()
endif()

message(STATUS "[Mia App] ex")

# Search source files:
# The examples app has a unitybuild mode
if (MIA_USE_UNITYBUILD)
    set(APP_EX_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/ex.c")
else ()
    file(GLOB APP_EX_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/*.c")
    list(REMOVE_ITEM APP_EX_SRCS "${CMAKE_CURRENT_LIST_DIR}/src/ex.c")
endif()

# Pass the source files list to mia
mia_app_sources(APP_EX_SRCS)


# /include dir is (if exists) added to the include_directories
# /res dir is (if exists) copied into the resources

# Let mia build this app
mia_app_build()


if(MIA_USE_APP_EX_MAIN_TEA)
    mia_app_logo("${CMAKE_CURRENT_LIST_DIR}/logo/ex_tea")
endif()

#
# App "hello" cmake script
#

option(MIA_USE_APP_HELLO "Build the hello world app" ON)


# only build the app if the option is set
if(NOT MIA_USE_APP_HELLO)
    return()
endif()

message(STATUS "[Mia App] hello")

# List all source files in this directory
file(GLOB APP_SRCS "${CMAKE_CURRENT_LIST_DIR}/*.c")

mia_app_sources(APP_SRCS)
mia_app_build()


# Install Mia on Linux (Ubuntu)
Back to [install](./install.md)

| Target       | Status                     | Notes                   |
|--------------|----------------------------|-------------------------|
| **SDL3**     | ✅ Supported               | Only via vendored build |
| **SDL2**     | ✅ Supported               | legacy                  |
| **Headless** | ✅ Supported               | with SDL3               |
| **WebApp**   | ✅ Supported               |                         |

## Table of Contents
- [Install](#install)
- [Run with SDL3](#run-with-sdl3)
- [Run Headless](#run-headless)
- [Run with SDL2 fallback](#run-with-sdl2-fallback)
- [WebApp](#webapp)

## Install
>Other distros may also work, but may need other calls to install.

```sh
#   update the system
sudo apt update && sudo apt upgrade
#   install basic stuff for c coding
sudo apt install build-essential gdb git cmake cmake-curses-gui
# curl (MIA_OPTION_FETCH)
sudo apt install libssl-dev
sudo apt install libcurl4-openssl-dev
```

### Install for SDL2 fallback
SDL2 is available via the package manager, SDL3 currently only via vendored built
```sh
### SDL2:
sudo apt install libsdl2-dev libsdl2-image-dev libsdl2-net-dev libsdl2-ttf-dev
#   install zenity for sfd file dialogs / picker
sudo apt install zenity
```

### Install for headless mode
For running Mia headless in a server while still being able to render stuff (cpu rendering)
```sh
sudo apt install xvfb
```



## Run with SDL3
```sh
#   clone Mia with vendored sdl3 
git clone --recursive git@github.com:renehorstmann/Mia.git
#
#   cd and make a build dir
cd Mia && mkdir build && cd build
#   run cmake debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DMIA_USE_VENDOR_SDL=true -DMIA_USE_VENDOR_SDL_IMAGE=true -DMIA_USE_VENDOR_SDL_NET=true -DMIA_USE_VENDOR_SDL_TTF=true
#   or to run cmake release:
#cmake .. -DCMAKE_BUILD_TYPE=Release -DMIA_USE_VENDOR_SDL=true -DMIA_USE_VENDOR_SDL_IMAGE=true -DMIA_USE_VENDOR_SDL_NET=true -DMIA_USE_VENDOR_SDL_TTF=true
#
#   to setup the Mia compilation:
#ccmake ..
#
#   compile with 16 threads
cmake --build . -- -j16
#
#   run
./Mia
```

## Run headless
Compile like with [Run with SDL3](#run-with-sdl3)
```sh
# Want to run in headless mode on a headless Ubuntu?
#    (typically sofware rendered...)
xvfb-run ./Mia
```


## Run with SDL2 fallback
In MacOs, Ubuntu, WSL Ubuntu or "MSYS2 MinGW x64" Terminal:
```sh
#   clone Mia
git clone git@github.com:renehorstmann/Mia.git
#   or clone with vendored sdl instead
#git clone --recursive git@github.com:renehorstmann/Mia.git
#
#   cd and make a build dir
cd Mia && mkdir build && cd build
#   run cmake debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DMIA_USE_SDL2=true
#   or to run cmake release:
#cmake .. -DCMAKE_BUILD_TYPE=Release -DMIA_USE_SDL2=true
#
#   to setup the Mia compilation:
#ccmake ..
#
#   compile with 16 threads
cmake --build . -- -j16
#
#   run
./Mia
```


## WebApp

Install the [Emscripten compiler](https://emscripten.org/docs/getting_started/downloads.html)

> The web version is based on SDL3 and needs the vendored built:
>  `git clone --recursive git@github.com:renehorstmann/Mia.git`

You should have already cloned the project and `cd` to that dir:

```sh
#   create the build directory
mkdir build_web && cd build_web
#
#   run cmake debug
emcmake cmake .. -DCMAKE_BUILD_TYPE=Debug
#   or to run cmake release:
#emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
#
#   to setup the Mia compilation:
#ccmake ..
#
#   compile
cmake --build . -- -j16
#
#   run (debug version) 
emrun index.html
```

- Another way to test the website (needed for release built) (open a browser and call localhost:8000)
```sh
python3 -m http.server --bind localhost  # [port]
```

- Clear the makefiles
```sh
# clear all but the webapp
find . ! -name "." ! -name ".." ! -name "debug.html" ! -name "favicon.ico" ! -name "icon*.png" ! -name "index*" -exec rm -rf {} +
```



Back to [install](./install.md)

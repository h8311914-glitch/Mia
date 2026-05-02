# Install Mia on MacOs
Back to [install](./install.md)

| Target     | Status                     | Notes  |
|------------|----------------------------|--------|
| **SDL3**   | ✅ Supported               |        |
| **SDL2**   | ✅ Supported               | legacy |
| **WebApp** | ✅ Supported               |        |

## Table of Contents
- [Install](#install)
- [Run with SDL3](#run-with-sdl3)
- [Run with SDL2 fallback](#run-with-sdl2-fallback)
- [WebApp](#webapp)


## Install
- Install XCode from the App Store. (This may be optional)
- Install [homebrew](https://brew.sh/) and add it to the path
- Install the needed libraries in the shell:
```sh
#   update the system
brew update && brew upgrade
#   install basic stuff for c coding
brew install git cmake
```

### Install for SDL3
```sh
### SDL3
brew install sdl3 sdl3_image
```

### Install for SDL2 fallback
```shell
### SDL2:
brew install sdl2 sdl2_image sdl2_net sdl2_ttf
```


## Run with SDL3
```sh
#   clone the repo
git clone git@github.com:renehorstmann/Mia.git
#   or clone with vendored sdl instead
#git clone --recursive git@github.com:renehorstmann/Mia.git
#
#   cd and make a build dir
cd Mia && mkdir build && cd build
#   run cmake debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
#   run cmake release
#cmake .. -DCMAKE_BUILD_TYPE=Release
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

## Run with SDL2 fallback
```sh
#   clone the repo
git clone git@github.com:renehorstmann/Mia.git
#   or clone with vendored sdl instead
#git clone --recursive git@github.com:renehorstmann/Mia.git
#
#   cd and make a build dir
cd Mia && mkdir build && cd build
#   run cmake debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DMIA_USE_SDL2=true
#   run cmake release
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

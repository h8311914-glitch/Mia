# Install Mia on Windows
Back to [install](./install.md)

| Platform       | Target     | Status            | Notes                        |
|----------------|------------|-------------------|------------------------------|
| **MSVC**       | **SDL3**   | ✅ Supported       |                              |
| **MSVC**       | **SDL2**   | ✅ Supported       | Legacy                       |
| **MSVC**       | **WebApp** | ❌ Unsupported     |                              |
| **WSL**        | **SDL3**   | ✅ Supported       | Only via vendored build      |
| **WSL**        | **SDL2**   | ✅ Supported       | Legacy                       |
| **WSL**        | **WebApp** | ✅ Supported       |                              |
| **MSYS2**      |            | ⚠️ Experimental   | May fail                     |

## Table of Contents
- [MSVC](#msvc)
  - [MSVC Install](#msvc-install)
  - [MSVC Run with SDL3](#msvc-run-with-sdl3)
  - [MSVC Run with SDL2 fallback](#msvc-run-with-sdl2-fallback)
- [WSL](#wsl)
- [MSYS2](#msys2)


## MSVC

### MSVC Install
MSVC is the Microsoft C(++) Compiler, that comes with Visual Studio.
The compiler can also be installed without the IDE (without needing the Visual Studio License...).

- install MSVC [Buildtools](https://visualstudio.microsoft.com/de/downloads/)
  - scroll down to "Tools for Visual Studio"
  - or install Visual Studio IDE directly if you want to use it
  - you need at least MSVC Version 2019 16.8
  - in the installer, check C++ development and the MSVC built tools + Windows SDK
- install a [git client](https://git-scm.com/download/win)
- install [cmake](https://cmake.org/download/) (add to system PATH)
- install [vcpkg](https://vcpkg.io/en/getting-started.html)
  - To install vcpk in PowerShell:
```sh
cd C:\\
#    or as a fallback in D:\\
mkdir dev
cd dev
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat -disableMetrics
cd vcpkg
```
- Install packages in Powershell
```sh
cd C:\\dev\vcpkg

# curl (MIA_OPTION_FETCH)
.\vcpkg install curl:x64-windows
```

#### Install for SDL3
```sh
### SDL3:
.\vcpkg install glew:x64-windows sdl3:x64-windows sdl3-image[core,jpeg,png]:x64-windows
```

#### Install for SDL2 fallback
```sh
### SDL2:
.\vcpkg install glew:x64-windows sdl2:x64-windows sdl2-image:x64-windows sdl2-net:x64-windows sdl2-ttf:x64-windows
```

### MSVC Run with SDL3
In the Powershell Terminal:
```sh
#   clone the repo
git clone git@github.com:renehorstmann/Mia.git
#   or clone with vendored sdl instead:
#git clone --recursive git@github.com:renehorstmann/Mia.git
#
#   cd and make a build dir
cd Mia 
mkdir build
cd build
#
#   run cmake debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
#   or to run cmake release:
#cmake .. -DCMAKE_BUILD_TYPE=Release
#
#   to setup the Mia compilation: run cmake gui
#
#   compile
cmake --build .
#   run
.\Debug\Mia.exe
```

### MSVC Run with SDL2 fallback
In the Powershell Terminal:
```sh
#   clone the repo
git clone git@github.com:renehorstmann/Mia.git
#   or clone with vendored sdl instead:
#git clone --recursive git@github.com:renehorstmann/Mia.git
#
#   cd and make a build dir
cd Mia 
mkdir build
cd build
#
#   run cmake debug
cmake .. -DCMAKE_BUILD_TYPE=Debug -DMIA_USE_SDL2=true
#   or to run cmake release:
#cmake .. -DCMAKE_BUILD_TYPE=Release -DMIA_USE_SDL2=true
#
#   to setup the Mia compilation: run cmake gui
#
#   compile
cmake --build .
#   run
.\Debug\Mia.exe
```

## WSL
With WSL (Windows Subsystem Linux) (needs Windows 11) 
you can use all the awesome unix tools in Windows with an Ubuntu terminal.
But you can't distribute your compiled programs (to Windows), the users would have to have WSL installed.
See [MSVC](#msvc), if you want to create an .exe to distribute for Windows

- run Powershell as admin
```
wsl --install
```
- Run The Ubuntu shell and follow [install_linux](./install_linux.md) to install and run


## MSYS2

> Warning, currently experimental / not supported, may fail
>
> Last time tested was with SDL2 a few years back

MSYS2 is a package manager for Windows, that can install the compiler Mingw-w64,
which is a port of the GCC-Compiler with wrapper POSIX headers for Windows.

- install [MSYS2](https://www.msys2.org/)
- update the package manager (as the site says: `pacman -Syu` ; restart ; `pacman -Su`)
- In "MSYS2 MSYS" Terminal
```sh
#   basic stuff for c coding
pacman -S --needed base-devel mingw-w64-x86_64-toolchain
pacman -S git mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
#   install sdl stuff
# 	  net, ttf are optional, see MIA_OPTION_SOCKET and MIA_OPTION_TTF
#
### SDL2:
pacman -S mingw-w64-x86_64-glew mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-SDL2_ttf
#
#   install curl (optional) for MIA_OPTION_FETCH 
pacman -S mingw-w64-x86_64-curl
```
- add the following line at the end of the file .bashrc (runs on terminal startup) (run `nano ~/.bashrc`)
```sh
# add mingw32 lib to the path, in this dir should be the opengl libraries (dll)
export PATH=$PATH:/c/msys64/mingw64/x86_64-w64-mingw32/lib
```
- Close the terminal
- If you use an IDE, you may need to add the PATH to ...mingw32/lib (see above) into the windows system variables


### MSYS2 Run
In the "MSYS2 MinGW x64" Terminal
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


Back to [install](./install.md)

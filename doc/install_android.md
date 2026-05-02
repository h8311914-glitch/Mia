# Install Mia on and for Android

Back to [install](./install.md)

| Target                                   | Status          | Notes              |
|------------------------------------------|-----------------|--------------------|
| **AndroidStudio App**                    | ✅ Supported     |                    |
| **CxxDroid (compile and run on mobile)** | ✅ Supported     | uses SDL2 fallback |
| **Termux (compile and run on mobile)**   | ⚠️ Experimental | May fail           |
| **WebApp (compile and run on mobile)**   | ✅ Supported     |                    |

## Table of Contents

- [AndroidStudio App](#androidstudio-app)
- [Install Termux](#install-termux)
- [CxxDroid](#cxxdroid)
- [Termux Compilation](#termux-compilation)
- [WebApp](#webapp)

## AndroidStudio App

Have a look at [Mia_Android](https://github.com/renehorstmann/Mia_Android)

## Install Termux
With Termux we can access Androids linux shell directly in an App + Termux has a package manager to install stuff.

- Install F-Droid: https://f-droid.org/
- In F-Droid, install Termux (https://termux.com/)

Open Termux:
```sh
#   update the system
pkg upgrade
#   install basic stuff
pkg i git
#   get access to the local file system of your android device from termux (needs the storage permission)
termux-setup-storage
# cd to your file system
cd ~/storage/shared
#   optional, create a c projects folder
mkdir cprojects && cd cprojects
#   clone Mia with vendored sdl3 
git clone --recursive git@github.com:renehorstmann/Mia.git
#   or to clone the repo without vendored built (CxxDroid only):
#git clone git@github.com:renehorstmann/Mia.git
```

### Tips for Termux usage
You can automate workflows using functions defined in the `~/.bashrc` file.
Like `cd` to the project dir, or git stuff (update and push, etc.).
Or to directly compile and launch Mia as webapp on your phone with a short call.
As an example:
```sh
# git pull
gpull() {
  git pull
}

# git add commit pull (merge) and push
gpush() {
  git add -A
  git commit -m "${1:-gpush}"
  git pull
  git push
}

# cd home
cdh() {
  cd ~/storage/shared
}

# cd home/GitProjects
cdg() {
  cdh
  cd GitProjects
}
```

## CxxDroid

> SDL3 is currently not supported

Useful to test and code on the go.

In fact, a lot of Mias's code, my games and apps were developed that way!

I recommend using a good Touchscreen-Keyboard-App with the right settings.
In my case its the Samsung default keyboard.
(Samsung Galaxy S22 Ultra; Samsung Galaxy Note 8; Samsung Galaxy Tab S6,
btw.).
My recommended settings for the Samsung Keyboard:

- Smart typing / Keybord swipe controls: **Cursor controls**
    - so its easy to navigate to the code with swipes instead of tipping on the line and position you want to editor
- Keyboard layout and feedback / Keyboard layout: **Number keys** checked; **Alternative characters** checked
    - so you can insert glyphs with a long press on the key, instead of switching to the alt. chars. site

I used the full/paid version of [CxxDroid](https://play.google.com/store/apps/details?id=ru.iiec.cxxdroid) (from Google
Play), other c compilers with SDL support may work, too

> To get the sources either just download Mia and unzip it, or use [Termux](#install-termux)
>
> To speed up CxxDroid usage, clone **without** the vendored stuff

In CxxDroid, install the following libraries (Menu/Install Libraries)
- SDL2
- SDL2-image
- SDL2-net (optional with MIA_OPTION_SOCKET)
- SDL2-ttf (optional with MIA_OPTION_TTF)
- libcurl (optional with MIA_OPTION_FETCH)

Then in CxxDroid to run:
- Open a file from Mia in CxxDroid
- You should now be able to compile and run directly on Android

### WARNING

**CxxDroid's build directory equals the source directory. So all generated files are mixed into the project.
These will get gitignored.**




## Termux Compilation

> Warning, currently experimental / not supported, may fail
> 
- Install Termux-X11: https://github.com/termux/termux-x11

- Follow the instructions to setup Termux-X11 and xfce4. For that, have a look at
  the [Droidmaster Tutorial](https://github.com/LinuxDroidMaster/Termux-Desktops/blob/main/Documentation/native/termux_native.md)

- In termux, install the needed libraries:

### Termux Compilation Install

```sh

#   if not already done, update the system
pkg upgrade
#   install basic stuff for c coding
pkg i build-essential gdb git cmake cmake-curses-gui
# curl (MIA_OPTION_FETCH)
pkg i curl
```
#### Termux Compilation Install for SDL2 fallback
```sh
### SDL2:
pkg i sdl2 sdl2-image sdl2-net sdl2-ttf
#   install zenity for sfd file dialogs / picker
pkg i zenity
```

### Termux Compilation Run with SDL3
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

### Termux Run with SDL2
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

> I had problems with setting up the sound. Seems like sdl's audio uses the available pulseaudio server. But on my phone
> it often started as dummy sink, so all was mute.

> My S-Pen works fine, but the virtual touch cursor does not work out of the box. Please reset the Touchscreen input
> mode to "Direct touch"



## WebApp

Open [Termux](#install-termux):
```sh
#   update the system
pkg upgrade
#   install basic stuff for c coding
pkg i build-essential gdb git cmake cmake-curses-gui
#   install emscripten
pkg i emscripten
```

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

### WebApp Tips
Use as with [Tips for Termux usage](#tips-for-termux-usage) .bashrc functions to compile and run the app
with a short fn name

"Install" the webpage to your android drawer to get a full screen app

Optionally use a split screen with the app on top and termux below to see logs and stuff.

To edit code either use:
- [CxxDroid](https://play.google.com/store/apps/details?id=ru.iiec.cxxdroid) (from Google
Play)
- [Squircle CE](https://github.com/massivemadness/Squircle-CE)
- [Acode](https://github.com/Acode-Foundation/Acode)
- or another text editor of your choice



Back to [install](./install.md)

# Mia's optional vendored libraries

The `vendor/` directory contains external libraries that are added as **Git submodules**. 
These libraries are used as dependencies in this project and can either be:

1. Vendored (via Git submodules in this case).
2. Linked as system-wide libraries (alternative / default, case it's way faster to compile).

### Submodules in Use

The following libraries are included as submodules in this directory:

- **SDL**: Simple DirectMedia Layer (core library for multimedia).
- **SDL_image**: An image loading library for SDL.
- **SDL_net**: Networking library for SDL.
- **SDL_ttf**: Font rendering library for SDL.

### Cloning the Repository with Submodules

If you forgot to clone the repository with submodules, 
you'll notice that this directory may be empty or incomplete. 
To properly initialize the submodules, use the following commands:

```bash
git submodule update --init --recursive
```

Alternatively, clone the repository with submodules from the start:

```bash
git clone --recursive git@github.com:renehorstmann/Mia.git
```

### Using Vendored Libraries

If you prefer to use the vendored libraries, update the cmake options MIA_USE_VENDORED_SDL*.

### Tips

- If you encounter build errors related to missing dependencies, ensure the submodules are initialized or that system libraries are correctly installed.
- Whenever the submodules are updated (e.g., new commits in the external libraries), update them in the local repository with:

```bash
git submodule update --remote
```
- The submodules are all forks of the official SDL repo. So Mia does not face versioning errors if SDL changes smth.

---

This file serves as a quick guide for anyone managing dependencies in this directory.

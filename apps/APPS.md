# Mia's apps

The `apps/` directory contains apps to be built by mia.

It's also possible to compile multiple apps at once (if globals do not crash and only a single main function exist).

Create and clone your own app/game repository in here. 

The apps directories must contain a `"app.cmake"` file that will be executed to gather source files, etc.

The contents of `res/` are automatically copied as mia app resources (if present)

The contents of `include/` is registered as include_directory (if present)

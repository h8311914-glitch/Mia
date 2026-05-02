# Mia Studio
ms Mia Studio itself is a collection of tools, working as smth like a uOS to start em.


## ToDo

‐ update event as a list
  - thumb update for xyz
  - dir changed (recursiv reload)
    - if failed step to home

- cloud
  - setting auto_download:
    - on
      - auto sync file list
      - auto download each file
      - default on all platforms except web
    - off
      - auto sync file list
      - click to download a file
      - default in emscripten / web
  - file local but not in cloud?
    - time local < cloud time
      - remove
    - else
      - upload
  - completely locked till synced?
  - states:
    - ready (up + down)
    - downloading
    - uploading
    - cloudonly

- fs
  - update time
    - updates parent dir times automatically
  - load / init
  - save?
    - auto_save?
  - terminal
    - $ ms ls
    - $ ms touch
    - $ ms exe
  - exe

- fs invalidate (if loading failed)
  - or just rm?



## Tools
- mp Mia Paint (easy to use pixel art editor)
- mc Mia Canvas (gpu pen drawing and edits)
- mt Mia Tiles (tile map editor)
- mw Mia Wave (sound editor)
- ...
- a camera app?

## Mia Studio app
The studio app is like a launcher for all tools and a file explorer.
Start tools with empty new projects.
Click on a file to open it with the corresponding tool.
(Will show a list of tools to choose from, like Paint or Canvas).
In the app "Mia " is removed from the names to save space.
If there is no need to save space it shows the full tool name.

## Online Cloud
As Mia Studio mainly only manages files and opens tools, its kinda seperated from the tools code.
The online cloud is a special directory in which all inner tree files are able to sync with the cloud.
Works with sub tires (as an example):
Free: 3 files, 10 MB, 2 ips / devices
Sub: inf files, 1 GB, 10 ips / devices
Click on Sync from (looks like an exe file cloud down) to sync from the cloud.
Will ask in merge conflicts (timestamp).
Click on Sync to (looks like an exe file cloud up) to sync to the cloud.
If files or space exceeds the limit, the sync will fail (up).

## App MockUp:
- title bar, settings (3 dots) on the right

- tools list to open empty stuff, single row

- last edit files list (timestamp), single row

- file explorer main view, flex box

the full app is scrollable.
if a dir is executed, it will show the file explorer in the full app.
then at the top the dir path is shown.

## Settings:
Most of em have a sync checkbox.

- display units or rescale.

- restart with last opened or main view.

- ...?

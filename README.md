# VR Base Station — STL Viewer

**EEEE2076 – Software Engineering & VR Project (Resit)**
Author: Ivan Huang

A Qt 6 desktop application for loading, inspecting and colouring STL models. Models
are listed in a tree view, rendered in an embedded VTK viewport, and can be displayed
on an HTC Vive headset through OpenVR.

---

## Features

- **Menu bar and toolbar** driven by Qt actions, with icons compiled in as Qt resources.
- **Open File** — pick a single `.stl` file; it is added to the tree and rendered immediately.
- **Open Directory** — pick a folder; every `.stl` inside is added to the tree. Sub-directories
  that contain STL files (at any depth) appear as their own child branches. Sub-directories
  with no STL files are skipped, so the tree does not fill up with empty folders.
- **Per-item properties** — right-click any tree item (or press <kbd>Ctrl</kbd>+<kbd>E</kbd>) to
  open a dialog for its name, visibility and colour. Colour can be set either with the
  R/G/B spin boxes or through the standard Qt colour chooser; changes are reflected in the
  3D view straight away.
- **Status bar** reporting the result of every action and the properties of the selected item.
- **VR view** using the HTC Vive via OpenVR.

---

## Prerequisites

| Dependency | Version used | Notes |
|---|---|---|
| CMake | 3.16 or newer | |
| Qt | 6.10.2 (`msvc2022_64`) | Needs the `Widgets` and `OpenGLWidgets` modules |
| VTK | 9.6 | Must be **built from source** — see below |
| OpenVR SDK | 1.x | Headers + `openvr_api.lib` |
| Compiler | MSVC 2022 or newer | |
| SteamVR | — | Only needed to actually run the VR view |

### VTK must be built with the right modules

The stock binary distributions of VTK do not include Qt support or the OpenVR back-end,
so VTK has to be compiled locally with at least these options enabled:

```
VTK_GROUP_ENABLE_Qt=YES
VTK_MODULE_ENABLE_VTK_GUISupportQt=YES
VTK_MODULE_ENABLE_VTK_RenderingOpenVR=YES
```

Build VTK in the **same configuration** (Debug or Release) that you intend to build this
project in — mixing a Debug VTK with a Release application will not link on MSVC.

---

## Building

```bash
git clone https://github.com/Ivanhuang2/ResitSoftware.git
cd ResitSoftware
```

Configure. `CMAKE_PREFIX_PATH` must point at your Qt and VTK installations:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64;C:/Program Files (x86)/VTK"
```

VTK's `RenderingOpenVR` module locates the OpenVR SDK through two cache variables. CMake
cannot guess these, so on a fresh build directory they must be supplied explicitly (adjust
the paths to wherever you unpacked the SDK):

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64;C:/Program Files (x86)/VTK" -DOpenVR_INCLUDE_DIR="C:/OpenVR/headers" -DOpenVR_LIBRARY="C:/OpenVR/lib/win64/openvr_api.lib"
```

> If configuration fails with `Could NOT find OpenVR (missing: OpenVR_LIBRARY OpenVR_INCLUDE_DIR)`,
> those two variables are what it is asking for.

Then build:

```bash
cmake --build build --config Debug
```

The executable is written to `build/Debug/VRBaseStation.exe`. The OpenVR controller binding
files in `vrbindings/` are copied next to the executable automatically as part of the build.

### Running from the build directory

The Qt and VTK runtime DLLs need to be findable. Either add them to `PATH`:

```bash
set PATH=C:\Qt\6.10.2\msvc2022_64\bin;C:\Program Files (x86)\VTK\bin;%PATH%
```

…or run `windeployqt` against the executable. Neither step is needed once the application
has been installed with the installer.

---

## Using the application

1. **Load a model.** *File → Open File* for a single STL, or *File → Open Directory* to pull in
   a whole folder tree at once. Loaded parts appear under the **Model** node.
2. **Navigate the 3D view.** Left-drag to rotate, right-drag or scroll to zoom, middle-drag to pan.
3. **Change a part's appearance.** Right-click it in the tree and choose *Item Options…*, then use
   *Choose Colour…* or the R/G/B boxes. Un-ticking **Visible** hides the part without removing it.
4. **Start VR.** Make sure SteamVR is running and the headset is tracking, then *VR → Start VR*.
   *VR → Stop VR* returns to the desktop view.

### Keyboard shortcuts

| Shortcut | Action |
|---|---|
| <kbd>Ctrl</kbd>+<kbd>O</kbd> | Open File |
| <kbd>Ctrl</kbd>+<kbd>D</kbd> | Open Directory |
| <kbd>Ctrl</kbd>+<kbd>E</kbd> | Item Options |
| <kbd>Ctrl</kbd>+<kbd>Q</kbd> | Quit |

---

## Project structure

| Path | Purpose |
|---|---|
| `main.cpp` | Entry point; sets the OpenGL surface format VTK requires before `QApplication` is created |
| `mainwindow.*` | Main window: actions, tree view, VTK viewport, status bar, file/directory loading |
| `ModelPart.*` | One node of the model tree; owns the STL reader, mapper and actor for a part |
| `ModelPartList.*` | `QAbstractItemModel` implementation backing the tree view |
| `optiondialog.*` | Property editor for a single part (name, colour, visibility) |
| `VRRenderThread.*` | Background thread that drives the OpenVR render loop |
| `icons.qrc`, `Icons/` | Toolbar and menu icons, compiled into the executable as Qt resources |
| `vrbindings/` | OpenVR controller binding descriptions, copied next to the executable at build time |

---

## Project status

- [x] GUI — menu bar, toolbar, resource icons, tree view, VTK viewport, status bar
- [x] Open File / Open Directory
- [x] Colour selection via the Qt colour chooser, reflected in the VTK view
- [ ] VR view (Start/Stop VR, actors and colours transferred to the headset)
- [ ] CMake/NSIS installer with start menu and desktop shortcuts
- [ ] Doxyfile

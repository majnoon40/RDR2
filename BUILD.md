# BUILD — compiling Terminus (b0rk3d V2) Terminus.dll on Windows

The project is an MSVC C++23 CMake project. Dependencies (ImGui, MinHook,
nlohmann/json, async-logger, RDR-classes, Vulkan headers) are pulled from
GitHub automatically by CMake's FetchContent at **configure** time, so you
need internet access during the first configure. No submodule init needed.

## 1. Prerequisites

- **Visual Studio 2022** with the *Desktop development with C++* workload
  (MSVC v143; the project requires C++23, which v143 handles).
- **CMake 3.20 or newer** — VS 2022 ships a bundled CMake, or install via
  `winget install Kitware.CMake`.
- Git (FetchContent clones dependencies).
- ~2 GB free disk for the fetched dependencies and build output.

## 2. Configure (fetches dependencies)

From the repo root, in a developer prompt (or use any plain prompt if cmake
is on PATH):

```bat
cmake -S . -B Build -A x64
```

First run downloads/clones the dependencies; if your network blocks GitHub,
configure will fail listing the missing repo.

Visual Studio alternative: *File → Open → Folder…* on the repo root
(`CMakeSettings.json` is included), then pick the x64-Release preset.

## 3. Build

```bat
cmake --build Build --config Release
```

Output: `Build/Release/Terminus.dll` (a MODULE library — that is the
injectable DLL). The repo also ships a prebuilt `Terminus.dll` at the root;
your freshly built one is what you should inject after making changes.

## 4. Use

Per the upstream README: inject **at the main menu, not in-game**;
the menu opens with **F5 / INSERT**. Inject with your injector of choice
into `RDR2.exe`.

## 5. Verify the enhancements

After building with `CHANGES.md` applied:

1. Open the menu → **Self → Main → Globals** — a new **Trainer Essentials**
   toggle should be the first item; flipping it on should light up God
   Mode, Keep Bars Filled, Keep Cores Filled, Infinite Ammo and Infinite
   Clip at once.
2. **Self → Horse → Globals** — a new **Keep Max Bonding** toggle under the
   horse options; with it on, your horse should be instantly max-bonded
   (check the horse's bonding under its name), and swapping horses should
   re-apply automatically.

## 6. Troubleshooting

- **CMake error about `cmake_minimum_required(VERSION 3.20.x)`** — that
  literal "x" in the upstream file is tolerated by CMake 3.20+; if your
  CMake rejects it, change the first line of `CMakeLists.txt` to
  `cmake_minimum_required(VERSION 3.20)`.
- **`vulkan-1.lib` not found** — it is vendored in `deps/`; build from the
  repo root so the relative path resolves.
- **LTCG/GUARD:NO linker errors** — those flags only apply under MSVC
  (`if(MSVC)` guard); make sure you are not building with Clang/GCC.

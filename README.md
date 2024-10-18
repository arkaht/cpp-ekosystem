# Ekosystem
**Ekosystem is a C++ 3D "game" focused around simulating a simple data-driven ecosystem of animals.**

It is built using [suprengine](https://github.com/arkaht/cpp-suprengine), my custom engine which uses OpenGL and SDL2.

## Dependencies
### Project
+ C++20 compiler
+ OpenGL 3.3.0
+ CMake 3.11
### Libraries
+ [cpp-suprengine](https://github.com/arkaht/cpp-suprengine)

## Features
+ A complete user interface tool using **ImGui** for game development, both system balancing and debug
+ TODO

## Project Structure
This project only holds the engine code since it is de-coupled from games code.

**Folder structure:**
+ **`assets/`** contains game assets, such as data defining pawns, curves, etc.
+ **`src/`** contains source files of the game.

## Build the project
This project is built using **CMake 3.11**, ensure you have already installed a compatible version.

### Steps
0. Create a folder anywhere for the installation, we'll name it `ekosystem`. The reason for this is that the game's `CMakeLists.txt` will attempt to finds the engine's folder directly from its own parent folder, if you don't want that, look at the [Troubleshooting](#troubleshooting) section.
1. Clone the engine's [repository](https://github.com/arkaht/cpp-suprengine) in `ekosystem/cpp-suprengine`.
2. Run command `git submodule update --init` in the engine's folder.
3. Clone the game's repository (this one) in `ekosystem/cpp-ekosystem`.
4. Run the game's `CMakeLists.txt` either by using CMake's command line interpreter, CMake's GUI or your favorite IDE.

### Troubleshooting

<details><summary><b>Change engine's folder location</b></summary>

If for some reasons you want to move the engine's or game's folder apart from each other,
you can configure the location of the engine using the CMake variable `SUPRENGINE_PATH`.

Using command line:
```cmd
cd build
cmake .. -DSUPRENGINE_PATH=C:/Path/To/Engine/
```

You can also use **cmake-gui** to change this variable.
</details>

<details><summary><b>Couldn't update engine's Git Submodules</b></summary>

If running the git submodule update command didn't work for any reasons, replace the folder `libs/curve-x` by cloning [arkaht/cpp-curve-x](https://github.com/arkaht/cpp-curve-x).
</details>

<details><summary><b>Need administrator privileges to run the CMakeLists.txt</b></summary>

TODO
</details>
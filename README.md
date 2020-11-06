## Gameplay Football
Football game, a fork of discontinued [GameplayFootball](https://github.com/BazkieBumpercar/GameplayFootball) written by [Bastiaan Konings Schuiling](http://www.properlydecent.com/).

In 2019, Google Brain team picked up a game and created a Reinforcement Learning environment based on it - [Google Research Football](https://github.com/google-research/football). They made some improvements to the game, updated the libraries, but threw away everything (e.g. menus, audio effects, etc.) that was not necessary for their task.

The goal of this repository is to update the existing code, based on Google Brain's changes (see `google_brain` branch) and other forks, and make it compiling and running on as many platforms as possible. PRs are always welcome.  

## Building from source

### Linux
Install required dependencies: 
```bash
sudo apt-get install git cmake build-essential libgl1-mesa-dev libsdl2-dev \
libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libopenal-dev libboost-all-dev \
libdirectfb-dev libst-dev mesa-utils xvfb x11vnc python3-pip
```

Run the following commands:
```bash
# Clone the repository
git clone https://github.com/vi3itor/GameplayFootball.git
cd GameplayFootball

# Copy the contents of `data` directory into `build`
cp -R data/. build

# Go to `build` directory
cd build
# Generate Makefile
cmake ..
# Compile the game
make -j$(nproc)
```

Run the game:
```bash
./gameplayfootball
```

### MacOS (Work in Progress)
**Important**: Currently, the game can be compiled on Mac OS, but it is not running yet, because rendering must be done on the Main Thread.

To install required dependencies you need [`brew`](https://brew.sh/) which can be installed in Terminal by running:
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
```

```bash
# Install dependencies
brew install git cmake sdl2 sdl2_image sdl2_ttf sdl2_gfx boost openal-soft
# Navigate to the directory where you want to put the repository
cd ~
# Clone the repository
git clone https://github.com/vi3itor/GameplayFootball.git
cd GameplayFootball
# Copy the contents of `data` directory into `build`
cp -R data/. build

# Go to `build` directory
cd build
# Generate Makefile
cmake ..
# Compile the game
make -j$(nproc)

# Run the game (Currently is not working)
./gameplayfootball
```



### Windows (Work in Progress)

Download and install:
- [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/),
- [Git](https://git-scm.com/download/win),
- [CMake](https://cmake.org/download/) (make sure to add it to the system PATH).

Install [`vcpkg`](https://github.com/microsoft/vcpkg) as explained in [Quick Start Guide](https://github.com/microsoft/vcpkg#quick-start-windows) or simply:
create a directory, e.g. `C:\dev`, open Command Prompt and run the following commands: 
```bat
% Navigate to the created directory
cd C:\dev

% Clone vckpg
git clone https://github.com/microsoft/vcpkg

% Run installation script
.\vcpkg\bootstrap-vcpkg.bat
```
Install required dependencies (all triplets **must be `x86-windows`**):
```bat 
.\vcpkg.exe install --triplet x86-windows boost:x86-windows sdl2 sdl2-image[libjpeg-turbo] sdl2-ttf sdl2-gfx opengl openal-soft
```

```bat
% Navigate to the directory where you want to put the repository
cd C:\dev

% Clone repository
git clone https://github.com/vi3itor/GameplayFootball.git 
cd GameplayFootball

% Switch to windows branch
git switch windows


% Copy the contents of `data` directory into `build\Debug` or (and) `build\Release`
xcopy /e /i data build\Debug
xcopy /e /i data build\Release
```
Go to `build` directory and generate `cmake` files. Make sure that you correctly set the directory for `vcpkg` (in our case it is installed into `C:\dev\vcpkg`):
```bat
cd build

cmake .. -DCMAKE_GENERATOR_PLATFORM=Win32 -DCMAKE_TOOLCHAIN_FILE=C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE  
```
To build `Release` version:
```bat
cmake --build . --parallel --config Release
```
For `Debug` version:
```bat
cmake --build . --parallel --config Debug
```

That's it! Run `gameplayfootball.exe` inside `build\Release` directory (or inside `build\Debug` for `Debug` version)


## Problems? 
If you have any problems please open an issue. 


### Donate
If you want to thank Bastiaan for his great work, consider a donation to his Bitcoin address 1JHnTe2QQj8RL281fXFiyvK9igj2VhPh2t

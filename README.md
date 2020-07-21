## Gameplay Football
Football game, a fork of discontinued [GameplayFootball]() written by [Bastiaan Konings Schuiling](http://www.properlydecent.com/).

In 2019, Google Brain team picked up a game and created a Reinforcement Learning environment based on it - [Google Research Football](https://github.com/google-research/football). They made some improvements to the game, updated the libraries, but threw away everything (e.g. menus) that was not necessary for their task.

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


### Donate
If you want to thank Bastiaan for his great work, consider a donation to his Bitcoin address 1JHnTe2QQj8RL281fXFiyvK9igj2VhPh2t

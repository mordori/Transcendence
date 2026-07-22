# Transcendence

## Pre-requisites

CMake, python3
``` bash
sudo apt update && sudo apt upgrade
sudo apt install cmake python3 -y
```

Emscripten
- Install emsdk in your common tools directory
``` bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

## Build
For now it's just a first playable with local player

``` bash
cd game
emcmake cmake -S . -B build
ln -sf build/compile_commands.json compile_commands.json
cmake --build build
```

## Run
Start local web server
``` python
python3 -m http.server -d build/client
```
Open in your web browser
```
http://localhost:8000/client.html
```

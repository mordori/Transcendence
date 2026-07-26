# Transcendence

## Pre-requisites

CMake, curl
``` bash
sudo apt update && sudo apt upgrade
sudo apt install cmake curl -y
```

Docker
``` bash
curl -fsSL https://get.docker.com -o get-docker.sh
sudo sh get-docker.sh
rm get-docker.sh
sudo usermod -aG docker $USER
newgrp docker
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
emcmake cmake -S game -B game/build
ln -sf game/build/compile_commands.json game/compile_commands.json
cmake --build game/build
```

Builds and start local web server
``` docker
docker build -t frontend frontend/
docker run -d -p 8080:80 --name frontend frontend
```

## Run
Open in your web browser
```
http://localhost:8080
```

To stop the web server
``` docker
docker stop frontend
```

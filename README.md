# Transcendence

## Pre-requisites
Make
``` bash
sudo apt update && sudo apt upgrade -y
sudo apt install make -y
```

CMake, curl, Docker, Emscripten
``` bash
make dependencies
```

## Build
> [!NOTE]
>
> For now it's just a first playable with local player

Light-weight static build content and proper services with Docker Compose
``` bash
make
```

or

Development oriented hot-reloading build with Vite
``` bash
make dev
```

## Run
Open in your web browser
```
http://localhost:8080
```

Stop the services
``` bash
make down
```

Start the pre-built containers
``` bash
make run
```

Delete build and Docker services
``` bash
make fclean
```

## Tech stack
### Game
- WebGPU, C++ Dawn headers
- Box3D
- enTT
- cgltf
- glaze
- glm

### Frontend
- Vite
- React
- Emscripten

### Backend
- .

### DevOps
- Docker
- NGINX

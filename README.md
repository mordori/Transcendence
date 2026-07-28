# Transcendence

## Pre-requisites
Assumes sudo and Make are installed

Installs CMake, curl, Docker, and Emscripten
``` bash
make dependencies
```

## Build
> [!NOTE]
>
> For now it's just a first playable with local player

Light-weight static content build and proper services using Docker Compose
``` bash
make
```

or

Development oriented hot-reloading build using Vite
``` bash
make dev
```

## Run
Open in your web browser
```
http://localhost
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

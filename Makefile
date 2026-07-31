SHELL := /bin/bash

BLUE		:=\033[1;34m
YELLOW		:=\033[1;33m
GREEN		:=\033[1;32m
RED			:=\033[1;31m
COLOR		:=\033[0m

ifeq ($(MAKELEVEL),0)
	MAKEFLAGS	+= --no-print-directory
endif

NAME	:=transcendence
PROC	:=-j$(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
PKG		:=$(shell command -v pacman >/dev/null 2>&1 && echo pacman || { command -v apt-get >/dev/null 2>&1 && echo apt || echo unknown; })

# The FMOD SDK cannot be committed: its EULA forbids redistributing SDK files.
FMOD_DIR	:=game/client/lib/fmod
FMOD_LIB	:=$(FMOD_DIR)/api/studio/lib/w32/fmodstudioL_wasm.a

all: build run

build: fmod-check
	@emcmake cmake -S game -B game/build
	@ln -sf build/compile_commands.json game/compile_commands.json
	@cmake --build game/build $(PROC)

fmod-check:
	@if [ ! -f "$(FMOD_LIB)" ]; then \
		echo; \
		echo -e "$(RED)FMOD SDK not found.$(COLOR)  Expected: $(FMOD_LIB)"; \
		echo; \
		echo -e "$(YELLOW)It cannot be committed - the FMOD EULA (1.3.iii) forbids"; \
		echo -e "redistributing SDK files. Free for educational use.$(COLOR)"; \
		echo; \
		echo -e "  1. Download the $(BLUE)HTML5$(COLOR) build of FMOD Engine:"; \
		echo -e "       $(BLUE)https://www.fmod.com/download$(COLOR)  (Engine -> HTML5)"; \
		echo -e "  2. Extract it so that this path exists:"; \
		echo    "       $(FMOD_DIR)/api/studio/lib/w32/"; \
		echo; \
		exit 1; \
	fi

run:
	@docker compose up -d --build
	@echo
	@echo -e "$(GREEN)✔$(COLOR)  Build successful and running: $(YELLOW)https://localhost/$(COLOR)"
	@echo

dev: build
	@cd frontend && npm run dev && cd ..

down:
	@docker compose down

logs:
	@docker compose logs -f

logs-server:
	@docker compose logs -f game-server

clean: down
	@rm -rf game/build
	@rm -f game/compile_commands.json

fclean: clean
	@docker rmi -f $$(docker images -q $(NAME)-*) 2>/dev/null || true
	@docker rmi -f $$(docker images -q frontend) 2>/dev/null || true
	@docker builder prune -af

re: fclean all

dependencies:
	@if [ "$(PKG)" = "unknown" ]; then \
		echo -e "$(RED)No supported package manager found (apt or pacman).$(COLOR)"; \
		exit 1; \
	fi
	@echo -e "$(BLUE)Using package manager: $(PKG)$(COLOR)"
	@if [ "$(PKG)" = "pacman" ]; then \
		sudo pacman -Syu --noconfirm; \
		sudo pacman -S --needed --noconfirm cmake curl git; \
	else \
		sudo apt update && sudo apt upgrade -y; \
		sudo apt install cmake curl git -y; \
	fi
	@if ! command -v docker >/dev/null 2>&1; then \
		echo -e "$(YELLOW)Installing Docker...$(COLOR)"; \
		if [ "$(PKG)" = "pacman" ]; then \
			sudo pacman -S --needed --noconfirm docker docker-compose docker-buildx; \
			sudo systemctl enable --now docker.service; \
		else \
			curl -fsSL https://get.docker.com -o get-docker.sh; \
			sudo sh get-docker.sh; \
			rm get-docker.sh; \
		fi; \
		sudo usermod -aG docker $$USER; \
	else \
		echo -e "$(GREEN)Docker is already installed.$(COLOR)"; \
	fi
	mkdir -p $$HOME/tools
	@if [ ! -d "$$HOME/tools/emsdk" ]; then \
		echo -e "$(YELLOW)Downloading emsdk...$(COLOR)"; \
		cd $$HOME/tools && git clone https://github.com/emscripten-core/emsdk.git; \
	else \
		echo -e "$(GREEN)emsdk already exists. Updating...$(COLOR)"; \
		cd $$HOME/tools/emsdk && git pull; \
	fi
	cd $$HOME/tools/emsdk && ./emsdk install latest && ./emsdk activate latest

	@RC_FILE=$$(if [[ $$SHELL == *"zsh"* ]]; then echo "$$HOME/.zshrc"; else echo "$$HOME/.bashrc"; fi); \
	if ! grep -q "emsdk_env.sh" "$$RC_FILE" 2>/dev/null; then \
		echo -e "\n# Emscripten paths" >> "$$RC_FILE"; \
		echo 'source $$HOME/tools/emsdk/emsdk_env.sh > /dev/null 2>&1' >> "$$RC_FILE"; \
	fi

	@echo
	@echo -e "$(GREEN)Dependencies installed!$(COLOR)"
	@echo -e "$(YELLOW)To apply the changes to your current terminal, run:$(COLOR)"
	@echo -e "  newgrp docker"
	@if [[ $$SHELL == *"zsh"* ]]; then \
		echo -e "  source ~/.zshrc"; \
	else \
		echo -e "  source ~/.bashrc"; \
	fi
	@echo

.PHONY: all build run dev dependencies fmod-check down logs logs-server clean fclean re

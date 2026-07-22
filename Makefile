NAME	:=transcendence

all: up

up:
	docker compose up -d --build

down:
	docker compose down

logs:
	docker compose logs -f

logs-server:
	docker compose logs -f game-server

clean: down
	docker system prune -af

fclean: clean
	docker rmi -f $$(docker images -q $(NAME)_*) 2>/dev/null || true
	docker system prune -f

re: fclean all

.PHONY: all up down logs logs-server clean fclean re

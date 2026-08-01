FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
	build-essential \
	cmake \
	ninja-build \
	git \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target game_server

FROM ubuntu:24.04

WORKDIR /app

COPY --from=builder /app/build/game_server /app/game_server

EXPOSE 8080

CMD ["./game_server"]

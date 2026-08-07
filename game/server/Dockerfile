FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
	build-essential \
	cmake \
	ninja-build \
	git \
	libssl-dev \
	zlib1g-dev

WORKDIR /src

COPY ./game ./game

WORKDIR /src/game

RUN cmake -B build -G Ninja
RUN cmake --build build --target server


FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
	libssl3 \
	zlib1g \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /src/game/build/server/server ./server

EXPOSE 9001

CMD ["./server"]

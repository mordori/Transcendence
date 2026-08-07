#include "network.hpp"

#include <algorithm>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "App.h"

namespace {
uWS::App* g_app = nullptr;
uWS::Loop* g_loop = nullptr;
std::mutex g_clientsMutex;
std::vector<uWS::WebSocket<false, true, int>*> g_clients;
std::thread g_networkThread;
}

namespace server::network {

void init(int port) {
	g_networkThread = std::thread([port]() {
		g_loop = uWS::Loop::get();
		uWS::App app;
		g_app = &app;

		app.ws<int>("/game",
			   { .open =
					   [](uWS::WebSocket<false, true, int>* ws) {
			std::cout << "[NETWORK] Client connected\n";
			std::lock_guard<std::mutex> lock(g_clientsMutex);
			g_clients.push_back(ws);
		},
				   .message =
					   [](uWS::WebSocket<false, true, int>* ws, std::string_view message, uWS::OpCode opCode) {
			// TODO: Pass message to match input queue
			// For now just log it
			(void)ws;
			(void)opCode;
			std::cout << "[NETWORK] Input received: " << message << "\n";
		},
				   .close =
					   [](uWS::WebSocket<false, true, int>* ws, int code, std::string_view message) {
			(void)code;
			(void)message;
			std::cout << "[NETWORK] Client disconnected\n";
			std::lock_guard<std::mutex> lock(g_clientsMutex);
			// Remove the disconnected client from our list
			g_clients.erase(std::remove(g_clients.begin(), g_clients.end(), ws), g_clients.end());
		} })
			.get("/heath",
				[](uWS::HttpResponse<false>* res, uWS::HttpRequest*) {
			res->end("OK");
		})

			.listen(port, [port](us_listen_socket_t* listenSocket) {
			if (listenSocket)
				std::cout << "[NETWORK] Listening on port " << port << "\n";
			else
				std::cerr << "[NETWORK] Failed to bind to port " << port << "\n";
		}).run();
	});
	g_networkThread.detach();
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void broadcast(const std::string& json) {
	if (!g_loop)
		return;

	std::string snapshot = json;

	g_loop->defer([snapshot = std::move(snapshot)]() {
		std::lock_guard<std::mutex> lock(g_clientsMutex);
		for (auto* ws : g_clients) {
			ws->send(snapshot, uWS::OpCode::TEXT);
		}
	});
}
}

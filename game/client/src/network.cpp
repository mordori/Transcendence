#include "network.hpp"

#include <emscripten/websocket.h>

#include <iostream>
#include <string>

namespace client::network {
EM_BOOL onOpen(int eventType, const EmscriptenWebSocketOpenEvent* websocketEvent, void* userData) {
	(void)eventType;
	(void)websocketEvent;
	(void)userData;
	std::cout << "[NETWORK] Connected to server!\n";
	return EM_TRUE;
}

EM_BOOL onError(int eventType, const EmscriptenWebSocketErrorEvent* websocketEvent, void* userData) {
	(void)eventType;
	(void)websocketEvent;
	(void)userData;
	std::cout << "[NETWORK] Connection error!\n";
	return EM_TRUE;
}

EM_BOOL onClose(int eventType, const EmscriptenWebSocketCloseEvent* websocketEvent, void* userData) {
	(void)eventType;
	(void)websocketEvent;
	(void)userData;
	std::cout << "[NETWORK] Disconnected from server!\n";
	return EM_TRUE;
}

EM_BOOL onMessage(int eventType, const EmscriptenWebSocketMessageEvent* websocketEvent, void* userData) {
	(void)eventType;
	(void)websocketEvent;
	(void)userData;
	if (websocketEvent->isText) {
		std::string message(reinterpret_cast<const char*>(websocketEvent->data), websocketEvent->numBytes);
		std::cout << "[NETWORK] Snapshot: " << message << "\n";
	}
	return EM_TRUE;
}

void setup(void* client) {
	if (!emscripten_websocket_is_supported()) {
		std::cerr << "[NETWORK] WebSockets not supported by the browser!\n";
		return;
	}

	EmscriptenWebSocketCreateAttributes ws_attributes{ "ws://127.0.0.1:9001/game", nullptr, EM_TRUE };

	EMSCRIPTEN_WEBSOCKET_T ws{ emscripten_websocket_new(&ws_attributes) };
	if (ws <= 0) {
		std::cerr << "[NETWORK] Failed to create WebSocket instance!\n";
		return;
	}

	emscripten_websocket_set_onopen_callback(ws, client, onOpen);
	emscripten_websocket_set_onerror_callback(ws, client, onError);
	emscripten_websocket_set_onclose_callback(ws, client, onClose);
	emscripten_websocket_set_onmessage_callback(ws, client, onMessage);
}
}

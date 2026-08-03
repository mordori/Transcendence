#pragma once

#include <cstdint>

#include "../match.hpp"

struct Match {
	core::match::State state{ core::match::State::LOBBY };
	float stateTimer{};
	float timer{};

	uint32_t scoreRed{};
	uint32_t scoreBlue{};
	uint32_t lastTeamToScore{};
};

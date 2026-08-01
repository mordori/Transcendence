#include "audio.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "components/events.hpp"
#include "fmod_errors.h"

namespace client::audio {

namespace {

bool fmodCheck(FMOD_RESULT result, const char* what) {
	if (result == FMOD_OK)
		return true;

	std::cerr << "[FMOD] " << what << " failed: " << FMOD_ErrorString(result) << '\n';
	return false;
}
}

void setup(entt::registry& registry) {
	FMOD::Debug_Initialize(FMOD_DEBUG_LEVEL_ERROR);

	FMOD::Studio::System* system{ nullptr };
	if (!fmodCheck(FMOD::Studio::System::create(&system), "System::create"))
		return;

	FMOD_RESULT result{ system->initialize(64, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr) };
	if (!fmodCheck(result, "System::initialize")) {
		system->release();
		return;
	}

	// The context owns everything that has to outlive setup.
	auto& audio{ registry.ctx().emplace<AudioContext>() };
	audio.system = system;
	system->getCoreSystem(&audio.core);

	fmodCheck(audio.core->createSound("/audio/test.wav", FMOD_DEFAULT, nullptr, &audio.hitSound),
		"createSound");

	std::cout << "[FMOD] initialized\n";
}

void update(entt::registry& registry, float deltaTime) {
	(void)deltaTime;

	auto* audio{ registry.ctx().find<AudioContext>() };
	auto* frame{ registry.ctx().find<FrameEvents>() };

	if (frame && audio && audio->hitSound) {
		for (const auto& impact : frame->impacts) {
			FMOD::Channel* channel{ nullptr };
			audio->core->playSound(audio->hitSound, nullptr, false, &channel);
			if (channel) {
				channel->setVolume(std::clamp(impact.speed / 20.0f, 0.1f, 1.0f));
				channel->setPitch(0.9f + (rand() % 200) / 1000.0f);
			}
		}
	}

	if (frame)
		frame->impacts.clear();

	if (audio && audio->system)
		audio->system->update();
}
}

#include "audio.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>

#include "components/events.hpp"
#include "components/physics.hpp"
#include "fmod_errors.h"
#include "glm/geometric.hpp"

namespace client::audio {

namespace {

// parameter reaches 1.0
constexpr float kMaxSpeed{ 40.0f };

bool fmodCheck(FMOD_RESULT result, const char* what) {
	if (result == FMOD_OK)
		return true;

	std::cerr << "[FMOD] " << what << " failed: " << FMOD_ErrorString(result) << '\n';
	return false;
}

void loadBanks(AudioContext& audio) {
	FMOD::Studio::Bank* bank{ nullptr };

	fmodCheck(audio.system->loadBankFile("/banks/Master.strings.bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL, &bank),
		"loadBankFile(Master.strings.bank)");

	fmodCheck(audio.system->loadBankFile("/banks/Master.bank",
				FMOD_STUDIO_LOAD_BANK_NORMAL, &bank),
		"loadBankFile(Master.bank)");
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

	auto& audio{ registry.ctx().emplace<AudioContext>() };
	audio.system = system;
	system->getCoreSystem(&audio.core);

	fmodCheck(audio.core->createSound("/audio/test.wav", FMOD_DEFAULT, nullptr, &audio.hitSound),
		"createSound");

	loadBanks(audio);

	std::cout << "[FMOD] initialized\n";
}

void attachEngine(entt::registry& registry, entt::entity entity) {
	auto* audio{ registry.ctx().find<AudioContext>() };
	if (!audio || !audio->system)
		return;

	FMOD::Studio::EventDescription* description{ nullptr };
	if (!fmodCheck(audio->system->getEvent("event:/engine", &description), "getEvent(engine)"))
		return;

	FMOD::Studio::EventInstance* instance{ nullptr };
	if (!fmodCheck(description->createInstance(&instance), "createInstance(car/engine)"))
		return;

	instance->start();
	registry.emplace_or_replace<AudioEmitter>(entity, instance);
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

	auto engineView = registry.view<const PlayerController, const AudioEmitter>();
	for (auto [entity, player, emitter] : engineView.each()) {
		if (!emitter.loop)
			continue;

		float speed{ glm::length(player.velocity) };
		emitter.loop->setParameterByName("speed", std::clamp(speed / kMaxSpeed, 0.0f, 1.0f));
	}

	if (audio && audio->system)
		audio->system->update();
}
}

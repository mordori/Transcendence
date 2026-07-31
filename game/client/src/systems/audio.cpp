#include "systems/audio.hpp"

#include "components/events.hpp"
#include "fmod_errors.h"

#include <algorithm>
#include <iostream>

namespace client::systems {

namespace {

bool    fmodCheck(FMOD_RESULT result, const char* what)
{
    if (result == FMOD_OK)
        return true;

    std::cerr << "[FMOD] " << what << " failed: " << FMOD_ErrorString(result) << '\n';
    return false;
}

}

void    setupAudio(entt::registry& registry)
{
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

    std::cout << "[FMOD] initialized\n";
}

void    updateAudio(entt::registry& registry, float dt)
{
    (void)dt;

    auto* audio{ registry.ctx().find<AudioContext>() };
    if (!audio || !audio->system)
        return;

    auto* frame{ registry.ctx().find<FrameEvents>() };
    if (frame && audio->hitSound) {
        for (const auto& impact : frame->impacts) {
            FMOD::Channel* channel{ nullptr };
            audio->core->playSound(audio->hitSound, nullptr, false, &channel);
            if (channel)
                channel->setVolume(std::clamp(impact.speed / 20.0f, 0.1f, 1.0f));
        }
    }

    audio->system->update();
}

}

#pragma once

#include "fmod.hpp"
#include "fmod_studio.hpp"

struct AudioContext {       // registry.ctx()
    FMOD::Studio::System* system{};
    FMOD::System* core{};   // owned by system, reached via getCoreSystem

    FMOD::Sound* hitSound{};

    FMOD::Studio::EventInstance* music{};
    FMOD::Studio::EventInstance* announcer{};
};

struct AudioEmitter {   // -> per entity
    FMOD::Studio::EventInstance* loop{}; // engine, ball roll
};

struct AudioListener {};

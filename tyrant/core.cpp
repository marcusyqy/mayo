#include "core.hpp"
#include <cassert>
#include <cstring>

void Clock::tick() {
    auto previous = latest;
    latest        = std::chrono::high_resolution_clock::now();
    dt            = std::chrono::duration_cast<std::chrono::duration<double>>(latest - previous).count();
}

Clock::Clock() : start(std::chrono::high_resolution_clock::now()), latest(start), dt{ 0.0 } {}

void IO::update(double dt) {
    for (int i = 0; i < Key_MAX; ++i) {
        if (key_states[i].key_down) {
            key_states[i].down_duration += dt;
        }
        // reset
        key_states[i].key_down = false;
    }
}

void IO::set_key_state(Key key, bool key_down) {
    assert(key < Key_MAX);
    key_states[key].key_down = key_down;
    if (!key_down) {
        key_states[key].previous_down_duration = key_states[key].down_duration;
        key_states[key].down_duration          = 0.0f;
    }
}

IO::IO() { std::memset(this, 0, sizeof(*this)); }

void Core::update() {
    clock.tick();
    io.update(clock.dt);
}

// what
Clock Core::clock = {};
IO Core::io       = {};

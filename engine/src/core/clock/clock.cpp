#include "clock.hpp"

#include "platform/platform.hpp"

namespace Engine {

    void Clock::Update() {
        if (this->start_time) {
            elapsed = Platform::GetAbsoluteTime() - this->start_time;
        }
    };

    void Clock::Start() {
        this->start_time = Platform::GetAbsoluteTime();
        this->elapsed = 0;
    };

    void Clock::Stop() {
        this->start_time = 0;
    };

    f64 Clock::GetElapsed() {
        return this->elapsed;
    };

    f64 Clock::GetStartTime() {
        return this->start_time;
    };

};
#pragma once

#include "nm/core/types.hpp"
#include <chrono>

namespace nm::core
{

class Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<f64>;

    Timer();

    void reset();
    void tick();

    [[nodiscard]] f64 getElapsedSeconds() const;
    [[nodiscard]] f64 getElapsedMilliseconds() const;
    [[nodiscard]] u64 getElapsedMicroseconds() const;

    [[nodiscard]] f64 getDeltaTime() const;

private:
    TimePoint m_startTime;
    TimePoint m_lastTickTime;
    f64 m_deltaTime;
};

} // namespace nm::core

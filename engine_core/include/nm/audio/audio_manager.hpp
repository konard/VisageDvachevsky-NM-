#pragma once

#include "nm/core/types.hpp"
#include "nm/core/result.hpp"

namespace nm::audio
{

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    Result<void> initialize();
    void shutdown();

    void playSound(const std::string& id, f32 volume = 1.0f);
    void stopAllSounds();

    void playMusic(const std::string& id, bool loop = true);
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void setMusicVolume(f32 volume);
    [[nodiscard]] bool isMusicPlaying() const;

    void setMasterVolume(f32 volume);
    [[nodiscard]] f32 getMasterVolume() const;

private:
    f32 m_masterVolume;
    f32 m_musicVolume;
    bool m_musicPlaying;
    bool m_initialized;
};

} // namespace nm::audio

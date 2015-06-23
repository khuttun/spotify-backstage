#ifndef SPOTIFY_BACKSTAGE_SOUNDSYSTEM_HPP
#define SPOTIFY_BACKSTAGE_SOUNDSYSTEM_HPP

#include <cstdint>
#include <memory>

namespace spotify_backstage {

struct EqState;

class SoundSystem
{
public:
    SoundSystem();
    ~SoundSystem();
    EqState getEqState();
    void flush();
    void setEqOn(bool on);
    void setGain(double gain);
    void setBass(double bass);
    void setMid(double mid);
    void setTreble(double treble);
    bool write(int sample_rate, int num_channels, const int16_t* data, int num_frames);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
#endif

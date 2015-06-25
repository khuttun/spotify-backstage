#ifndef SPOTIFY_BACKSTAGE_SOUNDSYSTEM_HPP
#define SPOTIFY_BACKSTAGE_SOUNDSYSTEM_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace spotify_backstage {

struct EqState;

class SoundSystem
{
public:
    SoundSystem();
    ~SoundSystem();
    int getCurrentOutputDevice();
    EqState getEqState();
    std::vector<std::pair<int, std::string>> getOutputDevices();
    void flush();
    void setEqOn(bool on);
    void setGain(double gain);
    void setBass(double bass);
    void setMid(double mid);
    void setTreble(double treble);
    void setOutputDevice(int dev);
    bool write(int sample_rate, int num_channels, const int16_t* data, int num_frames);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
#endif

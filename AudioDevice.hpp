#ifndef SPOTIFY_BACKSTAGE_AUDIODEVICE_HPP
#define SPOTIFY_BACKSTAGE_AUDIODEVICE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace spotify_backstage {

class AudioDevice
{
public:
    AudioDevice();
    ~AudioDevice();

    int getCurrentOutputDevice() const;
    std::vector<std::pair<int, std::string>> getOutputDevices() const;
    int getWriteAvailable() const;

    void flush();
    void setOutputDevice(int dev);
    int write(int sample_rate, int num_channels, const std::vector<int16_t>& audio_data);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}

#endif

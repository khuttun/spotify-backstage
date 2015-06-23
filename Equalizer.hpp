#ifndef SPOTIFY_BACKSTAGE_EQUALIZER_HPP
#define SPOTIFY_BACKSTAGE_EQUALIZER_HPP

#include <cstdint>
#include <memory>
#include <vector>

namespace spotify_backstage {

class AudioQueueEntry;

class Equalizer
{
public:
    Equalizer();
    ~Equalizer();

    double getGain() const;
    double getBass() const;
    double getMid() const;
    double getTreble() const;

    void equalize(std::vector<int16_t>& audio_data, int num_channels);
    void reset();
    void setGain(double gain);
    void setBass(double bass);
    void setMid(double mid);
    void setTreble(double treble);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
#endif

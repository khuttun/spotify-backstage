#ifndef SPOTIFY_BACKSTAGE_SPOTIFYBACKSTAGE_HPP
#define SPOTIFY_BACKSTAGE_SPOTIFYBACKSTAGE_HPP

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace spotify_backstage
{

struct EqState;
struct Track;

class SpotifyBackstage
{
public:
    SpotifyBackstage(const std::string& username, const std::string& password);

    ~SpotifyBackstage();

    EqState getEqState();

    std::vector<Track> getPlayQueue();

    void enqueue(const std::string& uri);

    void play();

    void stop();

    void next();

    std::vector<Track> search(const std::string& query, int num_results, int offset);

    void setEqOn(bool on);

    void setGain(double gain);

    void setBass(double bass);

    void setMid(double mid);

    void setTreble(double treble);

    int getCurrentOutputDevice();

    std::vector<std::pair<int, std::string>> getOutputDevices();

    void setOutputDevice(int dev);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

struct EqState
{
    EqState(bool on, double g, double b, double m, double t)
      : is_on(on), gain(g), bass(b), mid(m), treble(t)
    {
    }

    bool is_on;
    double gain;
    double bass;
    double mid;
    double treble;
};

struct Track
{
    Track(const std::string& artist_p, const std::string& album_p, const std::string& track_p,
        int duration_sec_p, const std::string& uri_p)
      : artist(artist_p), album(album_p), track(track_p), duration_sec(duration_sec_p), uri(uri_p)
    {
    }

    std::string artist;
    std::string album;
    std::string track;
    int duration_sec;
    std::string uri;
};

}

#endif

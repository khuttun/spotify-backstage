#ifndef SPOTIFY_BACKSTAGE_SPOTIFYSESSION_HPP
#define SPOTIFY_BACKSTAGE_SPOTIFYSESSION_HPP

#include <memory>
#include <string>
#include <vector>

namespace spotify_backstage {

class SoundSystem;
struct Track;

class SpotifySession
{
public:
    SpotifySession(const std::string& username, const std::string& password, SoundSystem& sounds);
    ~SpotifySession();
    std::vector<Track> getPlayQueue();
    void enqueue(const std::string& uri);
    void play();
    void stop();
    void next();
    std::vector<Track> search(const std::string& query, int num_results, int offset);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}
#endif

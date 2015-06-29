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

/**
 * SpotifyBackstage implements the API to spotify-backstage library.
 * It offers a Spotify-powered music backend including playback, queuing tracks, Spotify search, etc.
 * The API is used by creating a SpotifyBackstage instance.
 * 
 * Features:
 * - Play queue: Enqueue tracks to the play queue. spotify-backstage will play them in the order they were added.
 * - Search: Search tracks from Spotify catalog.
 * - Equalizer: Simple three channel equalizer.
 * - Output device selection: Ability to select the output device for the audio.
 */
class SpotifyBackstage
{
public:
    /**
     * Ctor.
     * 
     * @param username Spotify username
     * @param password Password for username
     */
    SpotifyBackstage(const std::string& username, const std::string& password);

    ~SpotifyBackstage();

    /** Get the current state of the equalizer */
    EqState getEqState();

    /** Get the current play queue */
    std::vector<Track> getPlayQueue();

    /**
     * Enqueue a track to the play queue.
     * 
     * @param uri Spotify URI of the track to enqueue.
     */
    void enqueue(const std::string& uri);

    /**
     * Start playback. The track currently at the head of the play queue will start playing.
     * Once the track is finished, it's removed from the play queue and the next track in the queue is played.
     * The playback will continue for as long as there are tracks in the play queue, or stop() is called.
     */
    void play();

    /** Stop playback. */
    void stop();

    /**
     * Stop the playback, remove the track currently at the head of the play queue from the queue,
     * and start playing the next track in the queue (if the queue is not empty).
     */
    void next();

    /**
     * Search tracks from the Spotify catalog.
     * 
     * @param query Search query string.
     * @param num_results Max number of tracks to return.
     * @param offset Offset in the full result list from which to start taking the returned tracks.
     *               This parameter is useful if you want to do more than one search with the same
     *               query string. For example,
     * 
     *               Get the first 10 results for "Neil Young":
     *               search("Neil Young", 10, 0);
     * 
     *               Get the next 10 results:
     *               search("Neil Young", 10, 10);
     */
    std::vector<Track> search(const std::string& query, int num_results, int offset);

    /**
     * Set equalizer on/off.
     * 
     * @param on true = on, false = off.
     */
    void setEqOn(bool on);

    /**
     * Set equalizer gain.
     * 
     * @gain The gain. 1.0 = 100%.
     */
    void setGain(double gain);

    /**
     * Set equalizer bass channel level.
     * 
     * @param bass The level. 1.0 = 100%.
     */
    void setBass(double bass);

    /**
     * Set equalizer mid channel level.
     * 
     * @param mid The level. 1.0 = 100%.
     */
    void setMid(double mid);

    /**
     * Set equalizer treble channel level.
     * 
     * @param treble The level. 1.0 = 100%.
     */
    void setTreble(double treble);

    /** Get the index of the currently selected audio output device. */
    int getCurrentOutputDevice();

    /**
     * Get a list of audio output devices available in the system.
     * The items in the list contain the device's index and name.
     */
    std::vector<std::pair<int, std::string>> getOutputDevices();

    /**
     * Set the audio output device.
     * 
     * @param dev Index of the device to set.
     */
    void setOutputDevice(int dev);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * EqState encapsulates the state of the equalizer.
 * With the gain and the channel levels, value 1.0 = 100%.
 */
struct EqState
{
    EqState(bool on, double g, double b, double m, double t)
      : is_on(on), gain(g), bass(b), mid(m), treble(t)
    {
    }

    /** Is the equalizer on? */
    bool is_on;
    
    /** Equalizer gain */
    double gain;
    
    /** Bass channel level */
    double bass;
    
    /** Mid channel level */
    double mid;
    
    /** Treble channel level */
    double treble;
};

/**
 * Track contains information of a single Spotify track.
 */
struct Track
{
    Track(const std::string& artist_p, const std::string& album_p, const std::string& track_p,
        int duration_sec_p, const std::string& uri_p)
      : artist(artist_p), album(album_p), track(track_p), duration_sec(duration_sec_p), uri(uri_p)
    {
    }

    /** Artist(s) */
    std::string artist;
    
    /** Album */
    std::string album;
    
    /** Track name */
    std::string track;
    
    /** Track duration in seconds */
    int duration_sec;
    
    /** Track's Spotify URI */
    std::string uri;
};

}

#endif

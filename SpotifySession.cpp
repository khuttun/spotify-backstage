#include "SpotifySession.hpp"

#include "Appkey.hpp"
#include "Logger.hpp"
#include "SoundSystem.hpp"
#include "SpotifyBackstage.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <libspotify/api.h>
#include <list>
#include <map>
#include <PolyM/Queue.hpp>
#include <sstream>
#include <thread>

// Check for Spotify errors
#define CHECK_SP_ERR(expr)\
    sp_error err_code = (expr);\
    if (err_code != SP_ERROR_OK)\
    {\
        LOG(sp_error_message(err_code));\
        exit(1);\
    }

namespace spotify_backstage {

namespace {

// Get Track object with data from sp_track and sp_link
Track getTrack(sp_track* track, sp_link* link)
{
    std::stringstream artistbuf;
    for (int i = 0; i < sp_track_num_artists(track); ++i)
    {
        if (i > 0)
            artistbuf << ", ";

        artistbuf << sp_artist_name(sp_track_artist(track, i));
    }

    char uri[128];
    sp_link_as_string(link, uri, 128);

    return Track(artistbuf.str(), sp_album_name(sp_track_album(track)), sp_track_name(track),
        sp_track_duration(track) / 1000, uri);
}

}

class SpotifySession::Impl
{
public:
    Impl(const std::string& username, const std::string& password, SoundSystem& sounds)
      : username_(username),
        password_(password),
        sounds_(sounds),
        msg_queue_(),
        spotify_cb_(),
        spotify_conf_(),
        spotify_(nullptr),
        play_queue_(),
        search_req_map_(),
        thread_(&Impl::run, this)
    {
    }

    ~Impl()
    {
        LOG("SpotifySession dtor");
        msg_queue_.put(PolyM::Msg(MSG_TERMINATE));
        thread_.join();
        LOG("Spotify thread finished");
    }

    std::vector<Track> getPlayQueue()
    {
        auto response = msg_queue_.request(PolyM::Msg(MSG_GET_PLAY_QUEUE));
        return dynamic_cast<PolyM::DataMsg<std::vector<Track>>&>(*response).getPayload();
    }

    void enqueue(const std::string& uri)
    {
        msg_queue_.put(PolyM::DataMsg<std::string>(MSG_ENQUEUE, uri));
    }

    void play()
    {
        msg_queue_.put(PolyM::Msg(MSG_PLAY));
    }

    void stop()
    {
        msg_queue_.put(PolyM::Msg(MSG_STOP));
    }

    void next()
    {
        msg_queue_.put(PolyM::Msg(MSG_NEXT));
    }

    std::vector<Track> search(const std::string& query, int num_results, int offset)
    {
        if (query.empty() || num_results < 1 || offset < 0)
        {
            LOG("Invalid search parameters: query = " << query << ", num_results = " <<
                num_results << ", offset = " << offset);
            return std::vector<Track>();
        }

        auto response = msg_queue_.request(
            PolyM::DataMsg<SearchQuery>(MSG_SEARCH, query, num_results, offset));
        return dynamic_cast<PolyM::DataMsg<std::vector<Track>>&>(*response).getPayload();
    }

private:
    // SpotifySession Msg types
    enum MsgType
    {
        MSG_TERMINATE,
        MSG_SPOTIFY_PROCESS,
        MSG_GET_PLAY_QUEUE,
        MSG_GET_PLAY_QUEUE_RESPONSE,
        MSG_ENQUEUE,
        MSG_PLAY,
        MSG_STOP,
        MSG_NEXT,
        MSG_SEARCH,
        MSG_SEARCH_RESPONSE
    };

    // Data passed in search message
    struct SearchQuery
    {
        SearchQuery(const std::string& query_p, int num_results_p, int offset_p)
          : query(query_p), num_results(num_results_p), offset(offset_p)
        {
        }
        std::string query;
        int num_results;
        int offset;
    };

    void run()
    {
        setupSpotify();

        auto keepRunning = true;
        auto msgReceiveTimeout = 0;
        while (keepRunning)
        {
            auto msg = msg_queue_.get(msgReceiveTimeout);
            switch (msg->getMsgId())
            {
            case MSG_TERMINATE:
                keepRunning = false;
                break;
            case MSG_SPOTIFY_PROCESS:
            case PolyM::MSG_TIMEOUT:
                msgReceiveTimeout = handleSpotifyProcess();
                break;
            case MSG_GET_PLAY_QUEUE:
                handleGetPlayQueue(msg->getUniqueId());
                break;
            case MSG_ENQUEUE:
                handleEnqueue(dynamic_cast<PolyM::DataMsg<std::string>&>(*msg).getPayload());
                break;
            case MSG_PLAY:
                handlePlay();
                break;
            case MSG_STOP:
                handleStop();
                break;
            case MSG_NEXT:
                handleNext();
                break;
            case MSG_SEARCH:
                handleSearch(dynamic_cast<PolyM::DataMsg<SearchQuery>&>(*msg));
                break;
            }
        }

        shutdownSpotify();
    }

    void setupSpotify()
    {
        // set up session callbacks
        std::memset(&spotify_cb_, 0, sizeof(spotify_cb_));
        spotify_cb_.logged_in = &loggedInCallback;
        spotify_cb_.notify_main_thread = &notifyMainCallback;
        spotify_cb_.music_delivery = &musicDeliveryCallback;
        spotify_cb_.end_of_track = &endOfTrackCallback;

        // set up session conf
        std::memset(&spotify_conf_, 0, sizeof(spotify_conf_));
        spotify_conf_.api_version = SPOTIFY_API_VERSION;
        spotify_conf_.cache_location = "";
        spotify_conf_.application_key = spotify_appkey;
        spotify_conf_.application_key_size = sizeof(spotify_appkey);
        spotify_conf_.user_agent = "spotify-backstage";
        spotify_conf_.callbacks = &spotify_cb_;
        spotify_conf_.userdata = this; // get reference to this in static callbacks

        sp_session_create(&spotify_conf_, &spotify_);
        sp_session_login(spotify_, username_.c_str(), password_.c_str(), false, NULL);

        LOG("Spotify session created");
    }

    void shutdownSpotify()
    {
        LOG("Shutting down Spotify");
        //sp_session_player_unload(spotify_);
        sp_session_logout(spotify_);
        sp_session_release(spotify_);
        LOG("Released libspotify resources");
    }

    // Msg handling functions

    int handleSpotifyProcess()
    {
        //LOG("handleSpotifyProcess");

        auto timeout = 0;
        do
        {
            sp_session_process_events(spotify_, &timeout);
        }
        while (timeout == 0);

        return timeout;
    }

    void handleGetPlayQueue(PolyM::MsgUID reqUid)
    {
        PolyM::DataMsg<std::vector<Track>> response(MSG_GET_PLAY_QUEUE_RESPONSE);

        for (const auto& link : play_queue_)
            response.getPayload().push_back(getTrack(sp_link_as_track(link), link));

        msg_queue_.respondTo(reqUid, std::move(response));
    }

    void handleEnqueue(const std::string& uri)
    {
        LOG("handleEnqueue " << uri);

        auto* link = sp_link_create_from_string(uri.c_str());

        if (!link)
        {
            LOG("Couldn't parse URI");
            return;
        }

        if (!sp_link_as_track(link))
        {
            LOG("URI not track");
            return;
        }

        play_queue_.push_back(link);
    }

    void handlePlay()
    {
        LOG("handlePlay");

        if (play_queue_.empty())
        {
            LOG("Play queue empty");
            return;
        }

        CHECK_SP_ERR(sp_session_player_load(spotify_, sp_link_as_track(play_queue_.front())));
        sp_session_player_play(spotify_, true);
    }

    void handleStop()
    {
        LOG("handleStop");
        sp_session_player_unload(spotify_);
        sounds_.flush();
    }

    void handleNext()
    {
        LOG("handleNext");

        if (play_queue_.empty())
        {
            LOG("Play queue empty");
            return;
        }

        handleStop();
        sp_link_release(play_queue_.front());
        play_queue_.pop_front();
        handlePlay();
    }

    void handleSearch(const PolyM::DataMsg<SearchQuery>& req)
    {
        const auto& query = req.getPayload();
        auto search = sp_search_create(spotify_, query.query.c_str(), query.offset, query.num_results,
            0, 0, 0, 0, 0, 0, SP_SEARCH_STANDARD, &searchCompleteCallback, this);
        LOG("Initiating search " << search << " with query " << query.query);
        search_req_map_[search] = req.getUniqueId();
    }

    // Callbacks coming from libspotify internal thread

    static void notifyMainCallback(sp_session* sp)
    {
        static_cast<Impl*>(sp_session_userdata(sp))->notifyMain();
    }

    static int musicDeliveryCallback(sp_session* sp, const sp_audioformat* format, const void* data, int num_frames)
    {
        return static_cast<Impl*>(sp_session_userdata(sp))->musicDelivery(format, data, num_frames);
    }

    static void endOfTrackCallback(sp_session* sp)
    {
        static_cast<Impl*>(sp_session_userdata(sp))->endOfTrack();
    }

    // Callbacks coming from main (message processing) thread

    static void loggedInCallback(sp_session*, sp_error error)
    {
        CHECK_SP_ERR(error);
        LOG("Logged in ok");
    }

    static void searchCompleteCallback(sp_search* search, void* userdata)
    {
        static_cast<Impl*>(userdata)->searchComplete(search);
    }

    // Implementations for the callbacks

    void notifyMain()
    {
        //LOG("notifyMain");
        msg_queue_.put(PolyM::Msg(MSG_SPOTIFY_PROCESS));
    }

    int musicDelivery(const sp_audioformat* format, const void* data, int num_frames)
    {
        //LOG("musicDelivery");
        return sounds_.write(
            format->sample_rate,
            format->channels,
            static_cast<const int16_t*>(data),
            num_frames) ? num_frames : 0;
    }

    void endOfTrack()
    {
        LOG("endOfTrack");
        next();
    }

    void searchComplete(sp_search* search)
    {
        LOG("Got results for search " << search);

        PolyM::DataMsg<std::vector<Track>> response(MSG_SEARCH_RESPONSE);

        for (int i = 0; i < sp_search_num_tracks(search); ++i)
        {
            auto track = sp_search_track(search, i);
            auto link = sp_link_create_from_track(track, 0);
            response.getPayload().push_back(getTrack(track, link));
            sp_link_release(link);
        }

        msg_queue_.respondTo(search_req_map_[search], std::move(response));

        search_req_map_.erase(search);
        sp_search_release(search);
    }

    const std::string& username_;
    const std::string& password_;
    SoundSystem& sounds_;
    PolyM::Queue msg_queue_;
    sp_session_callbacks spotify_cb_;
    sp_session_config spotify_conf_;
    sp_session* spotify_;
    std::list<sp_link*> play_queue_;
    std::map<sp_search*, PolyM::MsgUID> search_req_map_;
    std::thread thread_;
};

SpotifySession::SpotifySession(
    const std::string& username, const std::string& password, SoundSystem& sounds)
  : impl_(new Impl(username, password, sounds))
{
}

SpotifySession::~SpotifySession()
{
}

std::vector<Track> SpotifySession::getPlayQueue()
{
    return impl_->getPlayQueue();
}

void SpotifySession::enqueue(const std::string& uri)
{
    impl_->enqueue(uri);
}

void SpotifySession::play()
{
    impl_->play();
}

void SpotifySession::stop()
{
    impl_->stop();
}

void SpotifySession::next()
{
    impl_->next();
}

std::vector<Track> SpotifySession::search(
    const std::string& query, int num_results, int offset)
{
    return impl_->search(query, num_results, offset);
}

}

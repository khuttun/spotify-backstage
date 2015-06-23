#include "SpotifyBackstage.hpp"

#include "SoundSystem.hpp"
#include "SpotifySession.hpp"

namespace spotify_backstage
{

class SpotifyBackstage::Impl
{
public:
    Impl(const std::string& username, const std::string& password)
      : sounds_(), spotify_(username, password, sounds_)
    {
    }

    ~Impl()
    {
    }

    EqState getEqState()
    {
        return sounds_.getEqState();
    }

    std::vector<Track> getPlayQueue()
    {
        return spotify_.getPlayQueue();
    }

    void enqueue(const std::string& uri)
    {
        spotify_.enqueue(uri);
    }

    void play()
    {
        spotify_.play();
    }

    void stop()
    {
        spotify_.stop();
    }

    void next()
    {
        spotify_.next();
    }

    std::vector<Track> search(const std::string& query, int num_results, int offset)
    {
        return spotify_.search(query, num_results, offset);
    }

    void setEqOn(bool on)
    {
        sounds_.setEqOn(on);
    }

    void setGain(double gain)
    {
        sounds_.setGain(gain);
    }

    void setBass(double bass)
    {
        sounds_.setBass(bass);
    }

    void setMid(double mid)
    {
        sounds_.setMid(mid);
    }

    void setTreble(double treble)
    {
        sounds_.setTreble(treble);
    }

private:
    SoundSystem sounds_;
    SpotifySession spotify_;
};

SpotifyBackstage::SpotifyBackstage(const std::string& username, const std::string& password)
  : impl_(new Impl(username, password))
{
}

SpotifyBackstage::~SpotifyBackstage()
{
}

EqState SpotifyBackstage::getEqState()
{
    return impl_->getEqState();
}

std::vector<Track> SpotifyBackstage::getPlayQueue()
{
    return impl_->getPlayQueue();
}

void SpotifyBackstage::enqueue(const std::string& uri)
{
    impl_->enqueue(uri);
}

void SpotifyBackstage::play()
{
    impl_->play();
}

void SpotifyBackstage::stop()
{
    impl_->stop();
}

void SpotifyBackstage::next()
{
    impl_->next();
}

std::vector<Track> SpotifyBackstage::search(const std::string& query, int num_results, int offset)
{
    return impl_->search(query, num_results, offset);
}

void SpotifyBackstage::setEqOn(bool on)
{
    impl_->setEqOn(on);
}

void SpotifyBackstage::setGain(double gain)
{
    impl_->setGain(gain);
}

void SpotifyBackstage::setBass(double bass)
{
    impl_->setBass(bass);
}

void SpotifyBackstage::setMid(double mid)
{
    impl_->setMid(mid);
}

void SpotifyBackstage::setTreble(double treble)
{
    impl_->setTreble(treble);
}

}

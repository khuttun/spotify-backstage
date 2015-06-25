#include "SoundSystem.hpp"

#include "AudioDevice.hpp"
#include "Equalizer.hpp"
#include "Logger.hpp"
#include "SpotifyBackstage.hpp"
#include <thread>
#include <PolyM/Queue.hpp>

namespace spotify_backstage {

class SoundSystem::Impl
{
public:
    Impl()
      : audio_dev_(),
        eq_(),
        msg_queue_(),
        useEq_(false),
        thread_(&Impl::run, this)
    {
    }

    ~Impl()
    {
        LOG("SoundSystem dtor");
        msg_queue_.put(PolyM::Msg(MSG_TERMINATE));
        thread_.join();
        LOG("SoundSystem thread finished");
    }

    int getCurrentOutputDevice()
    {
        auto response = msg_queue_.request(PolyM::Msg(MSG_GET_CURRENT_OUTPUT_DEVICE));
        return dynamic_cast<PolyM::DataMsg<int>&>(*response).getPayload();
    }

    EqState getEqState()
    {
        auto response = msg_queue_.request(PolyM::Msg(MSG_GET_EQ_STATE));
        return dynamic_cast<PolyM::DataMsg<EqState>&>(*response).getPayload();
    }

    std::vector<std::pair<int, std::string>> getOutputDevices()
    {
        auto response = msg_queue_.request(PolyM::Msg(MSG_GET_OUTPUT_DEVICES));
        return dynamic_cast<PolyM::DataMsg<std::vector<std::pair<int, std::string>>>&>(*response).getPayload();
    }

    void flush()
    {
        msg_queue_.put(PolyM::Msg(MSG_FLUSH));
    }

    void setEqOn(bool on)
    {
        msg_queue_.put(PolyM::DataMsg<bool>(MSG_SET_EQ_ON, on));
    }

    void setGain(double gain)
    {
        msg_queue_.put(PolyM::DataMsg<double>(MSG_SET_GAIN, gain));
    }

    void setBass(double bass)
    {
        msg_queue_.put(PolyM::DataMsg<double>(MSG_SET_BASS, bass));
    }

    void setMid(double mid)
    {
        msg_queue_.put(PolyM::DataMsg<double>(MSG_SET_MID, mid));
    }

    void setTreble(double treble)
    {
        msg_queue_.put(PolyM::DataMsg<double>(MSG_SET_TREBLE, treble));
    }

    void setOutputDevice(int dev)
    {
        msg_queue_.put(PolyM::DataMsg<int>(MSG_SET_OUTPUT_DEVICE, dev));
    }

    bool write(int sample_rate, int num_channels, const int16_t* data, int num_frames)
    {
        auto response = msg_queue_.request(
            PolyM::DataMsg<Audio>(MSG_WRITE, sample_rate, num_channels, data, num_frames));
        return dynamic_cast<PolyM::DataMsg<bool>&>(*response).getPayload();
    }

private:
    // SoundSystem message types
    enum MsgType
    {
        MSG_TERMINATE,
        MSG_GET_CURRENT_OUTPUT_DEVICE,
        MSG_GET_CURRENT_OUTPUT_DEVICE_RESPONSE,
        MSG_GET_OUTPUT_DEVICES,
        MSG_GET_OUTPUT_DEVICES_RESPONSE,
        MSG_SET_OUTPUT_DEVICE,
        MSG_GET_EQ_STATE,
        MSG_GET_EQ_STATE_RESPONSE,
        MSG_SET_EQ_ON,
        MSG_SET_GAIN,
        MSG_SET_BASS,
        MSG_SET_MID,
        MSG_SET_TREBLE,
        MSG_WRITE,
        MSG_WRITE_RESPONSE,
        MSG_FLUSH
    };

    struct Audio
    {
        Audio(int fs, int n_ch, const int16_t* audio_data, int n_fr)
          : sample_rate(fs), num_channels(n_ch), data(audio_data, audio_data + n_ch * n_fr)
        {
        }

        int sample_rate;
        int num_channels;
        std::vector<int16_t> data;
    };

    void run()
    {
        auto keepRunning = true;
        while (keepRunning)
        {
            auto msg = msg_queue_.get();
            switch (msg->getMsgId())
            {
            case MSG_TERMINATE:
                keepRunning = false;
                break;
            case MSG_GET_CURRENT_OUTPUT_DEVICE:
                handleGetCurrentOutputDevice(msg->getUniqueId());
                break;
            case MSG_GET_OUTPUT_DEVICES:
                handleGetOutputDevices(msg->getUniqueId());
                break;
            case MSG_SET_OUTPUT_DEVICE:
                handleSetOutputDevice(dynamic_cast<PolyM::DataMsg<int>&>(*msg).getPayload());
                break;
            case MSG_GET_EQ_STATE:
                handleGetEqState(msg->getUniqueId());
                break;
            case MSG_SET_EQ_ON:
                handleSetEqOn(dynamic_cast<PolyM::DataMsg<bool>&>(*msg).getPayload());
                break;
            case MSG_SET_GAIN:
                handleSetGain(dynamic_cast<PolyM::DataMsg<double>&>(*msg).getPayload());
                break;
            case MSG_SET_BASS:
                handleSetBass(dynamic_cast<PolyM::DataMsg<double>&>(*msg).getPayload());
                break;
            case MSG_SET_MID:
                handleSetMid(dynamic_cast<PolyM::DataMsg<double>&>(*msg).getPayload());
                break;
            case MSG_SET_TREBLE:
                handleSetTreble(dynamic_cast<PolyM::DataMsg<double>&>(*msg).getPayload());
                break;
            case MSG_WRITE:
                handleWrite(dynamic_cast<PolyM::DataMsg<Audio>&>(*msg));
                break;
            case MSG_FLUSH:
                handleFlush();
                break;
            }
        }

        LOG("Shutting down SoundSystem");
    }

    void handleGetCurrentOutputDevice(PolyM::MsgUID reqUid)
    {
        msg_queue_.respondTo(reqUid, PolyM::DataMsg<int>(MSG_GET_CURRENT_OUTPUT_DEVICE_RESPONSE,
            audio_dev_.getCurrentOutputDevice()));
    }

    void handleGetOutputDevices(PolyM::MsgUID reqUid)
    {
        msg_queue_.respondTo(reqUid,
            PolyM::DataMsg<std::vector<std::pair<int, std::string>>>(
                MSG_GET_OUTPUT_DEVICES_RESPONSE, audio_dev_.getOutputDevices()));
    }

    void handleSetOutputDevice(int dev)
    {
        audio_dev_.setOutputDevice(dev);
    }

    void handleGetEqState(PolyM::MsgUID reqUid)
    {
        msg_queue_.respondTo(reqUid, PolyM::DataMsg<EqState>(MSG_GET_EQ_STATE_RESPONSE,
            useEq_, eq_.getGain(), eq_.getBass(), eq_.getMid(), eq_.getTreble()));
    }

    void handleSetEqOn(bool on)
    {
        useEq_ = on;
    }

    void handleSetGain(double gain)
    {
        eq_.setGain(gain);
    }

    void handleSetBass(double bass)
    {
        eq_.setBass(bass);
    }

    void handleSetMid(double mid)
    {
        eq_.setMid(mid);
    }

    void handleSetTreble(double treble)
    {
        eq_.setTreble(treble);
    }

    void handleWrite(PolyM::DataMsg<Audio>& msg)
    {
        //LOG(audio_dev_.getWriteAvailable());
        Audio& audio = msg.getPayload();
        if (audio_dev_.getWriteAvailable() >= static_cast<int>(audio.data.size()))
        {
            msg_queue_.respondTo(msg.getUniqueId(), PolyM::DataMsg<bool>(MSG_WRITE_RESPONSE, true));

            if (useEq_)
                eq_.equalize(audio.data, audio.num_channels);

            audio_dev_.write(audio.sample_rate, audio.num_channels, audio.data);
        }
        else
            msg_queue_.respondTo(msg.getUniqueId(), PolyM::DataMsg<bool>(MSG_WRITE_RESPONSE, false));
    }

    void handleFlush()
    {
        audio_dev_.flush();
    }

    AudioDevice audio_dev_;
    Equalizer eq_;
    PolyM::Queue msg_queue_;
    bool useEq_;
    std::thread thread_;
};

SoundSystem::SoundSystem()
  : impl_(new Impl)
{
}

SoundSystem::~SoundSystem()
{
}

int SoundSystem::getCurrentOutputDevice()
{
    return impl_->getCurrentOutputDevice();
}

EqState SoundSystem::getEqState()
{
    return impl_->getEqState();
}

std::vector<std::pair<int, std::string>> SoundSystem::getOutputDevices()
{
    return impl_->getOutputDevices();
}

void SoundSystem::flush()
{
    impl_->flush();
}

void SoundSystem::setEqOn(bool on)
{
    impl_->setEqOn(on);
}

void SoundSystem::setGain(double gain)
{
    impl_->setGain(gain);
}

void SoundSystem::setBass(double bass)
{
    impl_->setBass(bass);
}

void SoundSystem::setMid(double mid)
{
    impl_->setMid(mid);
}

void SoundSystem::setTreble(double treble)
{
    impl_->setTreble(treble);
}

void SoundSystem::setOutputDevice(int dev)
{
    impl_->setOutputDevice(dev);
}

bool SoundSystem::write(int sample_rate, int num_channels, const int16_t* data, int num_frames)
{
    return impl_->write(sample_rate, num_channels, data, num_frames);
}

}

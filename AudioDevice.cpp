#include "AudioDevice.hpp"

#include "Logger.hpp"
#include <boost/lockfree/spsc_queue.hpp>
#include <portaudio.h>
#include <cstdlib>
#include <thread>

#define CHECK_PA_ERR(expr)\
    {\
        PaError err = (expr);\
        if (err != paNoError)\
        {\
            LOG(Pa_GetErrorText(err));\
            exit(1);\
        }\
    }

namespace spotify_backstage {

namespace {
const int BUFFER_SIZE = 65536;
}

class AudioDevice::Impl
{
public:
    Impl()
      : stream_(nullptr), buffer_(BUFFER_SIZE), sample_rate_(44100), num_channels_(2), output_dev_(0)
    {
        CHECK_PA_ERR(Pa_Initialize());
        CHECK_PA_ERR(Pa_OpenDefaultStream(&stream_, 0, num_channels_, paInt16, sample_rate_, paFramesPerBufferUnspecified, staticCallback, this));
        output_dev_ = Pa_GetDefaultOutputDevice();
    }

    ~Impl()
    {
        CHECK_PA_ERR(Pa_CloseStream(stream_));
        CHECK_PA_ERR(Pa_Terminate());
    }

    int getCurrentOutputDevice() const
    {
        return output_dev_;
    }

    std::vector<std::pair<int, std::string>> getOutputDevices() const
    {
        std::vector<std::pair<int, std::string>> devices;

        for (int i = 0; i < Pa_GetDeviceCount(); ++i)
            if (Pa_GetDeviceInfo(i)->maxOutputChannels > 0)
                devices.push_back(std::make_pair(i, Pa_GetDeviceInfo(i)->name));

        return devices;
    }

    int getWriteAvailable() const
    {
        return buffer_.write_available();
    }

    void flush()
    {
        LOG("Flushing");
        stopStream();
        buffer_.reset();
    }

    void setOutputDevice(int dev)
    {
        LOG("Setting output device to " << dev);

        if (dev < 0 || dev >= Pa_GetDeviceCount())
        {
            LOG("Invalid device index");
            return;
        }

        LOG("Device name: " << Pa_GetDeviceInfo(dev)->name << ", max channels: " << Pa_GetDeviceInfo(dev)->maxOutputChannels);

        flush();
        reopenStream(sample_rate_, num_channels_, dev);
    }

    int write(int sample_rate, int num_channels, const std::vector<int16_t>& audio_data)
    {
        if (sample_rate != sample_rate_ || num_channels != num_channels_)
        {
            LOG("Change in sample rate / number of channels");
            flush();
            reopenStream(sample_rate, num_channels, output_dev_);
        }

        if (static_cast<int>(audio_data.size()) > getWriteAvailable())
        {
            LOG("Trying to write more than buffer can hold: " << audio_data.size());
            return 0;
        }

        const auto retval = buffer_.push(audio_data.data(), audio_data.size());
        startStream();

        return retval;
    }

private:
    // Start stream if it's not already started
    void startStream()
    {
        if (Pa_IsStreamStopped(stream_) && streamCanBeStarted())
        {
            CHECK_PA_ERR(Pa_StartStream(stream_));
        }
    }

    // Stop stream if it's active
    void stopStream()
    {
        if (Pa_IsStreamActive(stream_))
        {
            CHECK_PA_ERR(Pa_AbortStream(stream_));
        }
    }

    void reopenStream(int sample_rate, int num_channels, int dev)
    {
        LOG("Reopening stream. fs: " << sample_rate << ", channels: " << num_channels << ", dev: " << dev);

        PaStreamParameters params{};
        params.device = dev;
        params.channelCount = num_channels;
        params.sampleFormat = paInt16;
        params.suggestedLatency = 0.0;
        params.hostApiSpecificStreamInfo = nullptr;

        CHECK_PA_ERR(Pa_CloseStream(stream_));
        CHECK_PA_ERR(Pa_OpenStream(&stream_, nullptr, &params, sample_rate, paFramesPerBufferUnspecified, paNoFlag, staticCallback, this));

        sample_rate_ = sample_rate;
        num_channels_ = num_channels;
        output_dev_ = dev;
    }

    bool streamCanBeStarted() const
    {
        // allow starting the stream when we have enough data
        return buffer_.read_available() >= BUFFER_SIZE / 2;
    }

    int audioCallback(int16_t* outbuf, unsigned long num_frames_requested)
    {
        // This is the callback function run in the audio driver thread. It should run fast. No
        // - Locking
        // - Heavy filter calculations

        const auto num_samples = num_channels_ * num_frames_requested;
        const auto avail = buffer_.pop(outbuf, num_samples);
        if (avail != num_samples)
        {
            LOG("GLITCH: asked " << num_samples << ", got " << avail);
        }

        return paContinue;
    }

    static int staticCallback(
        const void*,
        void* outbuf,
        unsigned long num_frames_requested,
        const PaStreamCallbackTimeInfo*,
        PaStreamCallbackFlags,
        void* user_data)
    {
        return static_cast<Impl*>(user_data)->audioCallback(static_cast<int16_t*>(outbuf), num_frames_requested);
    }

    PaStream* stream_;
    boost::lockfree::spsc_queue<int16_t> buffer_;
    int sample_rate_;
    int num_channels_;
    int output_dev_;
};

AudioDevice::AudioDevice()
  : impl_(new Impl())
{
}

AudioDevice::~AudioDevice()
{
}

int AudioDevice::getCurrentOutputDevice() const
{
    return impl_->getCurrentOutputDevice();
}

std::vector<std::pair<int, std::string>> AudioDevice::getOutputDevices() const
{
    return impl_->getOutputDevices();
}

int AudioDevice::getWriteAvailable() const
{
    return impl_->getWriteAvailable();
}

void AudioDevice::flush()
{
    impl_->flush();
}

void AudioDevice::setOutputDevice(int dev)
{
    impl_->setOutputDevice(dev);
}

int AudioDevice::write(int sample_rate, int num_channels, const std::vector<int16_t>& audio_data)
{
    return impl_->write(sample_rate, num_channels, audio_data);
}

}

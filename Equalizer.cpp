#include "Equalizer.hpp"

#include "FilterBank.hpp"
#include "Logger.hpp"

namespace spotify_backstage {

namespace {
// Coefficients for 3rd order Butterworth filters used in the Equalizer
const std::vector<double> bassCoeffsA = { 1.0, -2.937170728449891, 2.876299723479332, -0.939098940325283 };
const std::vector<double> bassCoeffsB = { 3.7568380197861e-06, 1.12705140593583e-05, 1.12705140593583e-05, 3.7568380197861e-06 };
const std::vector<double> midCoeffsA = { 1.0, -4.460479873873305, 8.336970524397529, -8.443449445818612, 4.930457330101182, -1.575536490612530, 0.212045934326766 };
const std::vector<double> midCoeffsB = { 0.028635300170904, 0.0, -0.085905900512713, 0.0, 0.085905900512713, 0.0, -0.028635300170904 };
const std::vector<double> trebleCoeffsA = { 1.0, -1.459029062228061, 0.910369000290069, -0.197825187264320 };
const std::vector<double> trebleCoeffsB = { 0.445902906222806, -1.337708718668418, 1.337708718668418, -0.445902906222806 };
}

class Equalizer::Impl
{
public:
    Impl()
      : banks_(), gain_(1.0), bass_(1.0), mid_(1.0), treble_(1.0)
    {
        init(2);
    }
    
    ~Impl()
    {
    }

    void init(int num_channels)
    {
        banks_.clear();
        for (int i = 0; i < num_channels; ++i)
        {
            FilterBank bank;
            bank.addBand(BASS_ID, IirFilter(bassCoeffsB, bassCoeffsA));
            bank.addBand(MID_ID, IirFilter(midCoeffsB, midCoeffsA));
            bank.addBand(TREBLE_ID, IirFilter(trebleCoeffsB, trebleCoeffsA));
            banks_.push_back(bank);
        }

        setGain(1.0);
        setBass(1.0);
        setMid(1.0);
        setTreble(1.0);
    }
    
    double getGain() const
    {
        return gain_;
    }

    double getBass() const
    {
        return bass_;
    }

    double getMid() const
    {
        return mid_;
    }

    double getTreble() const
    {
        return treble_;
    }

    void equalize(std::vector<int16_t>& audio_data, int num_channels)
    {
        if (num_channels != static_cast<int>(banks_.size()))
        {
            LOG("Change in Equalizer channel count: " << num_channels);
            init(num_channels);
        }
        
        for (int i = 0; i < static_cast<int>(audio_data.size()); ++i)
            audio_data[i] = banks_[i % banks_.size()].filter(audio_data[i]);
    }
    
    void reset()
    {
        for (auto& bank : banks_)
            bank.reset();
    }
    
    void setGain(double gain)
    {
        gain_ = gain;
        for (auto& bank : banks_)
            bank.setOverallGain(gain_);
    }
    
    void setBass(double bass)
    {
        bass_ = bass;
        for (auto& bank : banks_)
            bank.setGain(BASS_ID, bass_);
    }
    
    void setMid(double mid)
    {
        mid_ = mid;
        for (auto& bank : banks_)
            bank.setGain(MID_ID, mid_);
    }
    
    void setTreble(double treble)
    {
        treble_ = treble;
        for (auto& bank : banks_)
            bank.setGain(TREBLE_ID, treble_);
    }

private:
    enum
    {
        BASS_ID,
        MID_ID,
        TREBLE_ID
    };

    std::vector<FilterBank> banks_;
    double gain_;
    double bass_;
    double mid_;
    double treble_;
};

Equalizer::Equalizer()
  : impl_(new Impl)
{
}
    
Equalizer::~Equalizer()
{
}

double Equalizer::getGain() const
{
    return impl_->getGain();
}

double Equalizer::getBass() const
{
    return impl_->getBass();
}

double Equalizer::getMid() const
{
    return impl_->getMid();
}

double Equalizer::getTreble() const
{
    return impl_->getTreble();
}

void Equalizer::equalize(std::vector<int16_t>& audio_data, int num_channels)
{
    impl_->equalize(audio_data, num_channels);
}

void Equalizer::reset()
{
    impl_->reset();
}

void Equalizer::setGain(double gain)
{
    impl_->setGain(gain);
}

void Equalizer::setBass(double bass)
{
    impl_->setBass(bass);
}

void Equalizer::setMid(double mid)
{
    impl_->setMid(mid);
}

void Equalizer::setTreble(double treble)
{
    impl_->setTreble(treble);
}

}

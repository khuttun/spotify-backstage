#include "FilterBank.hpp"

#include "Logger.hpp"

namespace spotify_backstage {

FilterBank::FilterBank()
  : bands_(), overallGain_(1.0)
{
}

FilterBank::~FilterBank()
{
}

void FilterBank::addBand(int band_id, const IirFilter& filter)
{
    bands_.push_back(Band(band_id, filter));
}

void FilterBank::setGain(int band_id, double gain)
{
    for (auto& band : bands_)
    {
        if (band.id == band_id)
        {
            band.gain = gain;
            return;
        }
    }
}

void FilterBank::setOverallGain(double gain)
{
	overallGain_ = gain;
}

void FilterBank::reset()
{
	for (auto& band : bands_)
	    band.filter.reset();
}
}

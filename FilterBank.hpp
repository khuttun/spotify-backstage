#ifndef SPOTIFY_BACKSTAGE_FILTERBANK_HPP
#define SPOTIFY_BACKSTAGE_FILTERBANK_HPP

#include "IirFilter.hpp"
#include <cstdint>
#include <limits>
#include <vector>

namespace spotify_backstage {

class FilterBank
{
public:
    FilterBank();
    ~FilterBank();

    void addBand(int band_id, const IirFilter& filter);
    void setGain(int band_id, double gain);
    void setOverallGain(double gain);
    int16_t filter(int16_t sample);
	void reset();

private:
	struct Band
	{
	    Band(int band_id, const IirFilter& f) : id(band_id), filter(f), gain(1.0) {}
		int id;
		IirFilter filter;
		double gain;
	};
	
	int16_t limitToRange(double val) const;

    std::vector<Band> bands_;
    double overallGain_;
};

inline int16_t FilterBank::filter(int16_t sample)
{
    auto result = 0.0;
    
    for (auto& band : bands_)
        result += band.gain * band.filter.filter(sample);

    result *= overallGain_;

	return limitToRange(result);
}

inline int16_t FilterBank::limitToRange(double val) const
{
    static auto min = std::numeric_limits<int16_t>::min();
    static auto max = std::numeric_limits<int16_t>::max();

    if (val < min) return min;
    if (val > max) return max;

    return static_cast<int16_t>(val);
}

}
#endif

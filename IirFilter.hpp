#ifndef SPOTIFY_BACKSTAGE_IIRFILTER_HPP
#define SPOTIFY_BACKSTAGE_IIRFILTER_HPP

#include <cstdint>
#include <vector>

namespace spotify_backstage {

class IirFilter
{
public:
    IirFilter(const std::vector<double>& b, const std::vector<double>& a);
    double filter(int16_t sample);
	void reset();

private:
    int getPrevIndex(int i) const;
    int getNextIndex(int i) const;
    
    // Zero/numerator coeffs
    std::vector<double> b_;
    // Pole/denominator coeffs
    std::vector<double> a_;
    // Delay line, used as circular buffer
    std::vector<double> w_;
    // Current index in wM
    int i_;
};

inline int IirFilter::getPrevIndex(int i) const
{
    return i > 0 ? i - 1 : w_.size() - 1;
}

inline int IirFilter::getNextIndex(int i) const
{
    return i < static_cast<int>(w_.size()) - 1 ? i + 1 : 0;
}

inline double IirFilter::filter(int16_t sample)
{
    // Implementation follows Direct Form II

    w_[i_] = static_cast<double>(sample);
    auto s = getPrevIndex(i_);
    for (int i = 1; i < static_cast<int>(a_.size()); ++i)
    {
        w_[i_] -= a_[i] * w_[s];
        s = getPrevIndex(s);
    }
    
    auto result = 0.0;
    s = i_;
    for (int i = 0; i < static_cast<int>(b_.size()); ++i)
    {
        result += b_[i] * w_[s];
        s = getPrevIndex(s);
    }
    
    i_ = getNextIndex(i_);
    
    return result;
}

}

#endif

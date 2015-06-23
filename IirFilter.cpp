#include "IirFilter.hpp"

#include "Logger.hpp"
#include <algorithm>
#include <cstdlib>

namespace spotify_backstage {

IirFilter::IirFilter(const std::vector<double>& b, const std::vector<double>& a)
  : b_(b), a_(a), w_(b.size() > a.size() ? b.size() : a.size(), 0.0), i_(0)
{
    if (a.size() > 0 && a[0] != 1.0)
    {
        LOG("Only normalized coefficients are supported");
        std::exit(1);
    }
}

void IirFilter::reset()
{
    std::fill(w_.begin(), w_.end(), 0.0);
    i_ = 0;
}

}

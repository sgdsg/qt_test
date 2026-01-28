
#include <complex>
#include <cstdint>
#include <vector>

template <class IN_T, class OUT_T, class TAP_T>
class fir_filter
{
public:
    fir_filter(const std::vector<TAP_T>& taps);

    void set_taps(const std::vector<TAP_T>& taps);
    unsigned int ntaps() const;

    OUT_T filter(const IN_T input[]) const;

protected:
    std::vector<TAP_T> d_taps;
    unsigned int d_ntaps;
};
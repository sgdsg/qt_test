
#include "fir_filter.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

template <class IN_T, class OUT_T, class TAP_T>
fir_filter<IN_T, OUT_T, TAP_T>::fir_filter(const std::vector<TAP_T>& taps)
{
    set_taps(taps);
}

template <class IN_T, class OUT_T, class TAP_T>
void fir_filter<IN_T, OUT_T, TAP_T>::set_taps(const std::vector<TAP_T>& taps)
{
    d_ntaps = (int)taps.size();
    d_taps = taps;
}

template <class IN_T, class OUT_T, class TAP_T>
unsigned int fir_filter<IN_T, OUT_T, TAP_T>::ntaps() const
{
    return d_ntaps;
}


template <class IN_T, class OUT_T, class TAP_T>
OUT_T fir_filter<IN_T, OUT_T, TAP_T>::filter(const IN_T input[]) const
{
    
}
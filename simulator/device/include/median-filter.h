#ifndef     MEDIAN_FILTER_H
#define     MEDIAN_FILTER_H

#include    <algorithm>
#include    <array>

#include    <device-export.h>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
template<size_t N>
class MedianFilter
{
    static_assert(N > 0 && N % 2 == 1, "Window size must be odd and positive");

public:

    double process(double input)
    {
        buffer[index] = input;
        index = (index + 1) % N;

        // Копируем текущее окно для сортировки
        std::array<double, N> temp = buffer;
        std::sort(temp.begin(), temp.end());

        return temp[N / 2];
    }

private:

    std::array<double, N> buffer{0.0f};
    size_t index = 0;
};

#endif

#ifndef     LOWPASS_FILTER_H
#define     LOWPASS_FILTER_H

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
class LowPassFilter
{
public:

    LowPassFilter(float alpha) : alpha_(alpha), initialized_(false) {}

    float process(float input)
    {
        if (!initialized_)
        {
            output_ = input;
            initialized_ = true;
        }
        else
        {
            output_ = alpha_ * input + (1.0f - alpha_) * output_;
        }

        return output_;
    }

private:

    float alpha_;       // 0 < alpha << 1 — чем меньше, тем сильнее сглаживание
    float output_ = 0.0f;
    bool initialized_ = false;
};

#endif

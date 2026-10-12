#pragma once

#include <JuceHeader.h>
#include <cmath>

namespace Clipper
{
    enum ClipShape
    {
        hardClip = 0,
        softClip
    };

    constexpr double softKnee = 0.7;

    inline double process(double inputSample, ClipShape shape)
    {
        if (shape == hardClip)
            return juce::jlimit(-1.0, 1.0, inputSample);

        const double magnitude = std::abs(inputSample);

        if (magnitude <= softKnee)
            return inputSample;

        const double normalizedMagnitude =
            (magnitude - softKnee) / (1.0 - softKnee);

        const double shapedMagnitude =
            softKnee + (1.0 - softKnee)
                     * std::tanh(normalizedMagnitude);

        return std::copysign(shapedMagnitude, inputSample);
    }
}

#pragma once

#include <JuceHeader.h>
#include <algorithm>
#include <cmath>
#include "EnvelopeDetector.h"

class BandCompressor
{
public:
    struct Params
    {
        double T = -6.0;           
        double W = 6.0;            
        double R = 4.0;            
        double attackSec = 0.01;   
        double tau_r_max = 0.2;    
        double alphaGC = 0.01;     
    };

    void prepare(double sampleRate)
    {
        fs = sampleRate;

        detector.prepare(fs, 50.0, 20.0);

        reset();
    }

    void reset()
    {
        detector.reset();

        gAggressive = 1.0;
        gNonAggr    = 1.0;
    }

    double process(double level ,double squaredLevel, const Params& p)
    {
        const auto detection = detector.process(
            level,
            squaredLevel,
            p.alphaGC
        );
        
        // values from peak detector
        const double crestFactor = detection.crestFactor;
        
        const double x_db = 20.0 * std::log10(std::max(level, 1e-12));
        double y_db;

        if (x_db < (p.T - p.W * 0.5))
            y_db = x_db;

        else if (x_db >= (p.T + p.W * 0.5))
            y_db = p.T + (x_db - p.T) / p.R;

        else
            y_db = x_db + (1.0 / p.R - 1.0) * std::pow(x_db - (p.T - p.W * 0.5), 2.0)
            / (2.0 * p.W);
        
        // back to linear for smoothing
        const double targetGain = std::pow(10.0, (y_db - x_db) / 20.0);
        
        
        
        // attack/release smoothing
        // Auto Release Based on crest Factor
        const double tau_r = std::max ((2.0 * p.tau_r_max) / (crestFactor * crestFactor) - p.attackSec, 0.03);
        const double tau_r_slow     = std::max (tau_r * 4.0, 0.3);
        
            
        const double a_attack       = 1.0 - std::exp (-1.0 / (fs * p.attackSec));
        const double a_release_aggr = 1.0 - std::exp (-1.0 / (fs * tau_r));
        const double a_release_slow = 1.0 - std::exp (-1.0 / (fs * tau_r_slow));
        
        if (targetGain < gAggressive)
            gAggressive += a_attack * (targetGain - gAggressive);
        else
            gAggressive += a_release_aggr * (targetGain - gAggressive);

    
        if (targetGain < gNonAggr)
            gNonAggr += a_attack * (targetGain - gNonAggr);
        else
            gNonAggr += a_release_slow * (targetGain - gNonAggr);
        
        //blend between both release profiles
        const double crestNorm = juce::jlimit (0.0, 1.0, (crestFactor - 1.0) / 9.0);
        return crestNorm * gAggressive + (1.0 - crestNorm) * gNonAggr;
    }

private:
    double fs = 44100.0;

    EnvelopeDetector detector;
    
    double gAggressive = 1.0;
    double gNonAggr    = 1.0;
};

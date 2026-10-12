#pragma once

#include <JuceHeader.h>

class EnvelopeDetector
{
public:
    struct Result
    {
        double peak = 0.0;
        double rms = 0.0;
        double crestFactor = 1.0;
    };

    void prepare(double sampleRate, double rmsTimeMs = 50.0, double peakDecayMs = 20.0)
    {
        fs = sampleRate;
        alphaRms  = 1.0 - std::exp (-1.0 / (fs * rmsTimeMs * 0.001));
        alphaPeak = std::exp (-1.0 / (fs * peakDecayMs * 0.001));
    }
    void reset()
    {
        peakEnvelope = 0; rmsSquared = 0; crestFactorSmooth = 1;
    }

    Result process(double level, double squaredLevel, double alphaCrest)
    {
        // Peak and RMS Detectors
        peakEnvelope = std::max(level, alphaPeak * peakEnvelope);
        
        rmsSquared += alphaRms * (squaredLevel - rmsSquared);
        const double rmsEnvelope = std::sqrt(rmsSquared + 1e-12);
        
        // Crest Factor 1.0 minimum
        const double crestFactorRaw = std::max (peakEnvelope / rmsEnvelope, 1.0);
        // Smoothed Crest factor controllable on UI
        crestFactorSmooth += alphaCrest * (crestFactorRaw - crestFactorSmooth);
        
        return Result {peakEnvelope,rmsEnvelope,crestFactorSmooth};
        
    }
    

private:
    double fs = 44100.0;

    // Envelope coefficients
    double alphaRms = 0.0;
    double alphaPeak = 0.0;

    // Detector state
    double peakEnvelope = 0.0;
    double rmsSquared = 0.0;
    double crestFactorSmooth = 1.0;
};

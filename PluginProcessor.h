/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "EnvelopeDetector.h"
#include "BandCompressor.h"
//==============================================================================
/**
*/
class AutolimitAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AutolimitAudioProcessor();
    ~AutolimitAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<float>* inputGainParam  = nullptr;
    std::atomic<float>* tauRMaxParam    = nullptr;
    std::atomic<float>* gcSmoothParam   = nullptr;
    std::atomic<float>* ceilingParam    = nullptr;
    std::atomic<float>* attackParam     = nullptr;
    std::atomic<float>* stereoLinkParam = nullptr;

    BandCompressor bandCompressor[2];

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutolimitAudioProcessor)
};

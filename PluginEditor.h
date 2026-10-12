/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class AutolimitAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AutolimitAudioProcessorEditor (AutolimitAudioProcessor&);
    ~AutolimitAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    void setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& name);


    AutolimitAudioProcessor& audioProcessor;

    juce::Slider inputGainKnob, attackKnob, tauRMaxKnob, gcSmoothKnob, ceilingKnob;
    juce::Label  inputGainLabel, attackLabel, tauRMaxLabel, gcSmoothLabel, ceilingLabel;

    juce::ToggleButton stereoLinkButton { "Stereo Link" };

    std::unique_ptr<SliderAttachment> inputGainAttachment, attackAttachment, tauRMaxAttachment,
                                      gcSmoothAttachment, ceilingAttachment;
    std::unique_ptr<ButtonAttachment> stereoLinkAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutolimitAudioProcessorEditor)
};

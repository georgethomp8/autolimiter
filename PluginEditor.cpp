/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AutolimitAudioProcessorEditor::AutolimitAudioProcessorEditor (AutolimitAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setupKnob(inputGainKnob, inputGainLabel, "Input Gain");
    setupKnob(attackKnob,    attackLabel,    "Attack");
    setupKnob(tauRMaxKnob,   tauRMaxLabel,   "Max Release");
    setupKnob(gcSmoothKnob,  gcSmoothLabel,  "Crest Smooth");
    setupKnob(ceilingKnob,   ceilingLabel,   "Ceiling");

    addAndMakeVisible(stereoLinkButton);

    auto& apvts = audioProcessor.apvts;

    inputGainAttachment  = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGainKnob);
    attackAttachment     = std::make_unique<SliderAttachment>(apvts, "attack",    attackKnob);
    tauRMaxAttachment    = std::make_unique<SliderAttachment>(apvts, "tauRMax",   tauRMaxKnob);
    gcSmoothAttachment   = std::make_unique<SliderAttachment>(apvts, "gcSmooth",  gcSmoothKnob);
    ceilingAttachment    = std::make_unique<SliderAttachment>(apvts, "ceiling",   ceilingKnob);
    stereoLinkAttachment = std::make_unique<ButtonAttachment>(apvts, "stereoLink", stereoLinkButton);

    setSize (600, 220);
}

AutolimitAudioProcessorEditor::~AutolimitAudioProcessorEditor()
{
}

void AutolimitAudioProcessorEditor::setupKnob(juce::Slider& knob, juce::Label& label, const juce::String& name)
{
    knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(knob);

    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

//==============================================================================
void AutolimitAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AutolimitAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);


    stereoLinkButton.setBounds(area.removeFromBottom(30).withSizeKeepingCentre(120, 30));

    juce::Slider* knobs[]  = { &inputGainKnob,  &attackKnob,  &tauRMaxKnob,  &gcSmoothKnob,  &ceilingKnob };
    juce::Label*  labels[] = { &inputGainLabel, &attackLabel, &tauRMaxLabel, &gcSmoothLabel, &ceilingLabel };

    const int knobWidth = area.getWidth() / 5;

    for (int i = 0; i < 5; i++)
    {
        auto column = area.removeFromLeft(knobWidth);
        labels[i]->setBounds(column.removeFromTop(20));
        knobs[i]->setBounds(column.reduced(5));
    }
}

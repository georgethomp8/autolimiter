/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EnvelopeDetector.h"
#include "BandCompressor.h"
#include "Clipper.h"

juce::AudioProcessorValueTreeState::ParameterLayout
AutolimitAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "inputGain", "Input Gain",
        juce::NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "tauRMax", "Max Release",
        juce::NormalisableRange<float> (100.0f, 2000.0f, 1.0f, 0.4f), 1000.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "gcSmooth", "Crest Smooth",
        juce::NormalisableRange<float> (10.0f, 500.0f, 1.0f, 0.4f), 200.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "ceiling", "Ceiling",
        juce::NormalisableRange<float> (-12.0f, 0.0f, 0.1f), -0.3f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));
    
    layout.add (std::make_unique<juce::AudioParameterFloat> (
        "attack", "Attack",
        juce::NormalisableRange<float> (0.1f, 100.0f, 0.1f, 0.5f), 10.0f,
        juce::AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<juce::AudioParameterBool> (
        "stereoLink", "Stereo Link", true));
    
    return layout;
}

//==============================================================================
AutolimitAudioProcessor::AutolimitAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    inputGainParam  = apvts.getRawParameterValue("inputGain");
    tauRMaxParam    = apvts.getRawParameterValue("tauRMax");
    gcSmoothParam   = apvts.getRawParameterValue("gcSmooth");
    ceilingParam    = apvts.getRawParameterValue("ceiling");
    attackParam     = apvts.getRawParameterValue("attack");
    stereoLinkParam = apvts.getRawParameterValue("stereoLink");

    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        2, 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true);
}

AutolimitAudioProcessor::~AutolimitAudioProcessor()
{
}

//==============================================================================
const juce::String AutolimitAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AutolimitAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AutolimitAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AutolimitAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AutolimitAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AutolimitAudioProcessor::getNumPrograms()
{
    return 1;
                
}

int AutolimitAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AutolimitAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AutolimitAudioProcessor::getProgramName (int index)
{
    return {};
}

void AutolimitAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AutolimitAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (auto& comp : bandCompressor)
        comp.prepare (sampleRate);

    oversampler->initProcessing((size_t) samplesPerBlock);
    oversampler->reset();

    
    setLatencySamples((int) oversampler->getLatencyInSamples());
}

void AutolimitAudioProcessor::releaseResources()
{

 
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AutolimitAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AutolimitAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const double fs = getSampleRate();

    const float inputGainDb = inputGainParam->load();
    const float tauRMaxMs   = tauRMaxParam->load();
    const float gcSmoothMs  = gcSmoothParam->load();
    const float ceilingDb   = ceilingParam->load();
    const float attackMs    = attackParam->load();
    const bool  stereoLink  = stereoLinkParam->load() > 0.5f;

    // dB -> linear
    const double inputGain = juce::Decibels::decibelsToGain((double) inputGainDb);
    const double ceiling   = juce::Decibels::decibelsToGain((double) ceilingDb);

    BandCompressor::Params p;
    p.tau_r_max = tauRMaxMs * 0.001;
    p.attackSec = attackMs * 0.001;
    p.alphaGC   = 1.0 - std::exp(-1.0 / (fs * gcSmoothMs * 0.001));

    const int numChannels = juce::jmin(totalNumInputChannels, 2);
    const int numSamples  = buffer.getNumSamples();

    for (int n = 0; n < numSamples; n++)
    {
        double left  = buffer.getSample(0, n) * inputGain;
        double right = left;

        if (numChannels > 1)
            right = buffer.getSample(1, n) * inputGain;

        // comp
        double gainL = 1.0;
        double gainR = 1.0;

        if (stereoLink)
        {
            const double level = std::max(std::abs(left), std::abs(right));
            gainL = bandCompressor[0].process(level, level * level, p);
            gainR = gainL;
        }
        else
        {
            gainL = bandCompressor[0].process(std::abs(left),  left * left,   p);
            gainR = bandCompressor[1].process(std::abs(right), right * right, p);
        }

        left  *= gainL;
        right *= gainR;

        buffer.setSample(0, n, (float) left);

        if (numChannels > 1)
            buffer.setSample(1, n, (float) right);
    }

    // clipper
    juce::dsp::AudioBlock<float> block (buffer);
    auto clipBlock = block.getSubsetChannelBlock(0, (size_t) numChannels);
    auto upBlock   = oversampler->processSamplesUp(clipBlock);

    const int upSamples = (int) upBlock.getNumSamples();

    for (int ch = 0; ch < numChannels; ch++)
    {
        auto* data = upBlock.getChannelPointer((size_t) ch);

        for (int n = 0; n < upSamples; n++)
            data[n] = (float) (ceiling * Clipper::process(data[n] / ceiling, Clipper::softClip));
    }

    oversampler->processSamplesDown(clipBlock);

    for (int ch = 0; ch < numChannels; ch++)
    {
        auto* data = buffer.getWritePointer(ch);

        for (int n = 0; n < numSamples; n++)
            data[n] = (float) (ceiling * Clipper::process(data[n] / ceiling, Clipper::hardClip));
    }
}

//==============================================================================
bool AutolimitAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AutolimitAudioProcessor::createEditor()
{
    return new AutolimitAudioProcessorEditor (*this);
}

//==============================================================================
void AutolimitAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void AutolimitAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutolimitAudioProcessor();
}

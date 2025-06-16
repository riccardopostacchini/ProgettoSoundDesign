#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EasyRecAudioProcessor::EasyRecAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}
EasyRecAudioProcessor::~EasyRecAudioProcessor()
{
}

//==============================================================================
const juce::String EasyRecAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EasyRecAudioProcessor::acceptsMidi() const
{
   return false;
}

bool EasyRecAudioProcessor::producesMidi() const
{
   return false;
}

bool EasyRecAudioProcessor::isMidiEffect() const
{
    return false;
}

double EasyRecAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EasyRecAudioProcessor::getNumPrograms()
{
    return 1;
}

int EasyRecAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EasyRecAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EasyRecAudioProcessor::getProgramName (int index)
{
    return {};
}

void EasyRecAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EasyRecAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    lowCutFilter.reset();
    lowCutFilter.prepare(spec);
    lowCutFilter.state = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 20.0f);

    toneShelfFilter.reset();
    toneShelfFilter.prepare(spec);
    toneShelfFilter.state = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 8000.0f, 0.707f, 1.0f);
}

void EasyRecAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EasyRecAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Supporta solo stereo in e stereo out
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}
#endif

void EasyRecAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());


    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
    }
    
    juce::dsp::AudioBlock<float> block (buffer);

        // Processa il low cut
        lowCutFilter.process(juce::dsp::ProcessContextReplacing<float>(block));

        // Processa il tone shelving
        toneShelfFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
}

void EasyRecAudioProcessor::updateEQFilters(float lowCutFreq, float toneAmount)
{
    auto sampleRate = getSampleRate();

    // cutoff low cut da 20 a 200 Hz
    lowCutFreq = juce::jlimit(20.0f, 200.0f, lowCutFreq);
    *lowCutFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowCutFreq);

    // toneAmount 0..1 mappato su guadagno shelving 0..+12 dB
    float gainDb = juce::jmap(toneAmount, 0.0f, 1.0f, 0.0f, 12.0f);
    float gainLinear = juce::Decibels::decibelsToGain(gainDb);

    *toneShelfFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 8000.0f, 0.707f, gainLinear);
}
//==============================================================================
bool EasyRecAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* EasyRecAudioProcessor::createEditor()
{
    return new EasyRecAudioProcessorEditor (*this);
}

//==============================================================================
void EasyRecAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    
}

void EasyRecAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{

}


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EasyRecAudioProcessor();
}

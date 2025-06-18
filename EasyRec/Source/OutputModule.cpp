#include "OutputModule.h"

OutputModule::OutputModule() {}

OutputModule::~OutputModule() {}

void OutputModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    // Preparazioni eventuali (per ora nessuna, ma puoi usarlo se serve)
    // Spec può essere usato se in futuro vuoi allocare buffer o simili
}

void OutputModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    auto numChannels = buffer.getNumChannels();
    auto numSamples = buffer.getNumSamples();

    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);

        for (int i = 0; i < numSamples; ++i)
        {
            channelData[i] *= gain;
        }
    }
}

void OutputModule::setGainDb(float gainDb)
{
    gain = juce::Decibels::decibelsToGain(gainDb);
}
void OutputModule::reset()
{
    // nessuna struttura dsp da resettare per ora
}

#include "SaturationModule.h"

SaturationModule::SaturationModule() {}

SaturationModule::~SaturationModule() {}

void SaturationModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float)spec.sampleRate;
    // Qui eventualmente inizializza altri parametri o buffer se serve
}

void SaturationModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float sample = channelData[i];
            processSample(sample);
            // Mix dry/wet in base a amount (0..1)
            channelData[i] = channelData[i] * (1.0f - amount) + sample * amount;
        }
    }
}

void SaturationModule::processSample(float& sample)
{
    // Esempio semplice di saturazione soft clipping
    if (softMode)
    {
        // Soft clipping con tanh
        sample = std::tanh(sample * 5.0f);
    }
    else
    {
        // Hard clipping semplice
        if (sample > 0.6f) sample = 0.6f;
        else if (sample < -0.6f) sample = -0.6f;
    }
}

void SaturationModule::setAmount(float newAmount)
{
    amount = juce::jlimit(0.0f, 1.0f, newAmount);
}

void SaturationModule::setSoftMode(bool soft)
{
    softMode = soft;
}
void SaturationModule::reset()
{
    // Non serve fare reset a strutture DSP, ma se vuoi pulire variabili:
    amount = 0.0f;
}

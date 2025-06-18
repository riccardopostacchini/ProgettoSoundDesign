#pragma once
#include <JuceHeader.h>

class SaturationModule
{
public:
    SaturationModule();
    ~SaturationModule();

    // Cambiato prepare per usare ProcessSpec
    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setAmount(float amount);
    void setSoftMode(bool soft);
    void reset();

private:
    float amount = 0.0f;
    bool softMode = true;
    float sampleRate = 44100.0f;

    // Funzione di saturazione semplice su un campione
    void processSample(float& sample);
};

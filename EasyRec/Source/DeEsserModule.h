#pragma once
#include <JuceHeader.h>

class DeEsserModule
{
public:
    DeEsserModule();
    ~DeEsserModule();

    // Aggiornata la firma di prepare
    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setAmount(float amount);
    void reset();
private:
    juce::dsp::IIR::Filter<float> bandpassFilter;
    juce::dsp::Compressor<float> compressor;
    float sampleRate = 44100.0f;

    float amount = 0.0f; // 0..1
};

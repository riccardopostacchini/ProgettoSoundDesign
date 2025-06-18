#pragma once
#include <JuceHeader.h>

class EQModule
{
public:
    EQModule();
    ~EQModule();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setLowCutFreq(float freq);
    void setToneAmount(float amount);
    void reset();
private:
    juce::dsp::ProcessorChain<
        juce::dsp::IIR::Filter<float>,  // Low cut
        juce::dsp::IIR::Filter<float>   // Tone filter (peak/shelf)
    > filterChain;

    float sampleRate = 44100.0f;
};

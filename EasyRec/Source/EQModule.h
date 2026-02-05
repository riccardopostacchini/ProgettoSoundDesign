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
    using IIRFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    juce::dsp::ProcessorChain<
        IIRFilter,  // Low cut (1)
        IIRFilter,  // Low cut (2) -> 24 dB/ott
        IIRFilter,  // Low cut (3)
        IIRFilter,  // Low cut (4) -> 48 dB/ott
        IIRFilter,  // Low cut (5)
        IIRFilter,  // Low cut (6) -> 72 dB/ott
        IIRFilter,  // Low cut (7)
        IIRFilter,  // Low cut (8) -> 96 dB/ott
        IIRFilter   // Tone filter (peak/shelf)
    > filterChain;

    float sampleRate = 44100.0f;
    juce::SmoothedValue<float> lowCutSmoothed;
    float lastLowCutHz = 110.0f;

    juce::SmoothedValue<float> toneSmoothedDb;
    float lastToneDb = 0.0f;
};

#pragma once
#include <JuceHeader.h>

class OutputModule
{
public:
    OutputModule();
    ~OutputModule();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setGainDb(float gainDb);
    void reset();

private:
    float gain = 1.0f;
};

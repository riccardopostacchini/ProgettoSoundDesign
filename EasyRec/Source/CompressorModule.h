#pragma once
#include <JuceHeader.h>

class CompressorModule
{
public:
    CompressorModule();
    ~CompressorModule();

    // Cambiato il prepare per accettare direttamente ProcessSpec
    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setAmount(float amount);
    void setSoftMode(bool soft);
    void reset();
private:
    float sampleRate = 44100.0f;
    juce::dsp::Compressor<float> compressor;

    bool softMode = true;
};


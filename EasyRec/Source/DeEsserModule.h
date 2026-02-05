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
    using IIRFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    IIRFilter bandpassFilter; // banda sibilanti per split-band
    juce::dsp::Compressor<float> compressor;
    float sampleRate = 44100.0f;

    float amount = 0.0f; // 0..1
    juce::AudioBuffer<float> dryBuffer;
    juce::AudioBuffer<float> bandBuffer;
    juce::AudioBuffer<float> detectBuffer;

    static constexpr int numDetectBands = 4;
    const float detectFreqs[numDetectBands] = { 4000.0f, 5500.0f, 7000.0f, 9000.0f };
    IIRFilter detectFilters[numDetectBands];
    float currentCenterHz = 6000.0f;
};

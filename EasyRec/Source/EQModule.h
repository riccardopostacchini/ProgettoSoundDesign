#pragma once
#include <JuceHeader.h>

class EQModule
{
public:
    EQModule();
    ~EQModule();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

    void setBassAmount(float bassAmountDb);
    void setTrebleAmount(float trebleAmountDb);
    void setSoftPreset(bool softPreset);
    void reset();

private:
    using IIRFilter = juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>>;

    juce::dsp::ProcessorChain<
        IIRFilter,  // Rumble roll-off < 80 Hz
        IIRFilter,  // Low shelf (body)
        IIRFilter,  // Mud control peak (~280 Hz)
        IIRFilter,  // High shelf (air)
        IIRFilter   // Presence peak
    > filterChain;

    float sampleRate = 44100.0f;
    bool isSoftPreset = true;
    juce::SmoothedValue<float> bassSmoothedDb;
    juce::SmoothedValue<float> trebleSmoothedDb;
    float lastBassDb = 0.0f;
    float lastTrebleDb = 0.0f;
    bool forceUpdate = true;

    void updateFilters(float bassDb, float trebleDb);
};

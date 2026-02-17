#pragma once
#include <JuceHeader.h>

class CompressorModule
{
public:
    CompressorModule();
    ~CompressorModule();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void processBlock(juce::AudioBuffer<float>& buffer);

    // amountDb: [-10 .. +10], controlla la quantita' di gain reduction
    void setAmount(float amountDb);
    // inputDb: [-10 .. +10], aumenta/riduce la pressione sul compressore
    void setInputDriveDb(float inputDb);
    void setSoftMode(bool soft);
    void reset();

private:
    float sampleRate = 44100.0f;
    float amountDb = 0.0f;
    float inputDriveDb = 0.0f;
    bool softMode = true;

    // Parametri attivi del preset
    float ratio = 4.0f;
    float thresholdDb = -18.0f;
    float attackMs = 5.0f;
    float releaseMs = 50.0f;
    float kneeDb = 8.0f; // soft knee > 0, hard knee = 0

    // Stato smoothing gain
    float gainReductionDbSmoothed = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    void updateTimeConstants();
    float computeGainReductionDb(float levelDb) const;
};


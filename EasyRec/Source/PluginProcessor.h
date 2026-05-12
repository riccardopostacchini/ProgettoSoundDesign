#pragma once

#include <JuceHeader.h>
#include <atomic>
#include "EQModule.h"
#include "CompressorModule.h"
#include "SaturationModule.h"
#include "OutputModule.h"

class EasyRecAudioProcessor  : public juce::AudioProcessor
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;

    EasyRecAudioProcessor();
    ~EasyRecAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void updateEQFilters(float bassAmountDb, float trebleAmountDb);
    void setCompressorAmount(float amount);
    void setCompressorSoftMode(bool soft);
    void setSaturationAmount(float amount);
    void setSaturationSoftMode(bool soft);
    void setOutputGainDb(float gainDb);

    APVTS& getAPVTS() { return parameters; }
    float getOutputMeterDb() const noexcept { return outputMeterDb.load(); }
    float getInputMeterDb() const noexcept { return inputMeterDb.load(); }

private:
    static APVTS::ParameterLayout createParameterLayout();

    APVTS parameters;

    EQModule eq;
    CompressorModule compressor;
    SaturationModule saturation;
    OutputModule output;

    // Screen 2 FX (CLA-style fixed engines with one send each)
    juce::dsp::Reverb roomReverb;
    juce::dsp::Reverb churchReverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> roomPreDelayL { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> roomPreDelayR { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> churchPreDelayL { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> churchPreDelayR { 96000 };
    juce::dsp::IIR::Filter<float> roomHpL, roomHpR, roomLpL, roomLpR;
    juce::dsp::IIR::Filter<float> churchHpL, churchHpR, churchLpL, churchLpR;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> slapDelayL { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> slapDelayR { 96000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> eighthDelayL { 192000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> eighthDelayR { 192000 };
    double currentSampleRate = 44100.0;
    std::atomic<float> outputMeterDb { -60.0f };
    std::atomic<float> inputMeterDb { -60.0f };

    // Hidden de-esser interno (broadband reduction guidata dalla banda sibilante)
    juce::dsp::IIR::Filter<float> deEsserBandL;
    juce::dsp::IIR::Filter<float> deEsserBandR;
    float deEssEnv = 0.0f;
    float deEssAttackCoeff = 0.0f;
    float deEssReleaseCoeff = 0.0f;
    float deEssGainSmoothed = 1.0f;
    float deEssGainAttackCoeff = 0.0f;
    float deEssGainReleaseCoeff = 0.0f;

    void processHiddenDeEsser(juce::AudioBuffer<float>& buffer);

    // Hidden safety tone shaping (sub/ultra-high containment)
    juce::dsp::IIR::Filter<float> safetyHpL;
    juce::dsp::IIR::Filter<float> safetyHpR;
    juce::dsp::IIR::Filter<float> safetyLpL;
    juce::dsp::IIR::Filter<float> safetyLpR;
    void processHiddenSafetyFilters(juce::AudioBuffer<float>& buffer);

    // Hidden peak protector (gentle limiter + soft clip)
    float hiddenLimiterGain = 1.0f;
    float hiddenLimiterAttackCoeff = 0.0f;
    float hiddenLimiterReleaseCoeff = 0.0f;
    void processHiddenPeakProtector(juce::AudioBuffer<float>& buffer);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyRecAudioProcessor)
};

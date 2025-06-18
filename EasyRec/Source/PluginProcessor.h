#pragma once

#include <JuceHeader.h>
#include "EQModule.h"
#include "DeEsserModule.h"
#include "CompressorModule.h"
#include "SaturationModule.h"
#include "OutputModule.h"

class EasyRecAudioProcessor  : public juce::AudioProcessor
{
public:
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
    
    void updateEQFilters(float lowCutFreq, float toneAmount);
    void setDeEsserAmount(float amount);
    void setCompressorAmount(float amount);
    void setCompressorSoftMode(bool soft);
    void setSaturationAmount(float amount);
    void setSaturationSoftMode(bool soft);
    void setOutputGainDb(float gainDb);

private:
    EQModule eq;
    DeEsserModule deEsser;
    CompressorModule compressor;
    SaturationModule saturation;
    OutputModule output;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyRecAudioProcessor)
};

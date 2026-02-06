#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EasyRecAudioProcessor::EasyRecAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
                       ),
       parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#else
     : AudioProcessor (BusesProperties()),
       parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

EasyRecAudioProcessor::~EasyRecAudioProcessor()
{
}

//==============================================================================
const juce::String EasyRecAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EasyRecAudioProcessor::acceptsMidi() const
{
    return false;
}

bool EasyRecAudioProcessor::producesMidi() const
{
    return false;
}

bool EasyRecAudioProcessor::isMidiEffect() const
{
    return false;
}

double EasyRecAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EasyRecAudioProcessor::getNumPrograms()
{
    return 1;
}

int EasyRecAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EasyRecAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EasyRecAudioProcessor::getProgramName (int index)
{
    return {};
}

void EasyRecAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EasyRecAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    eq.prepare(spec);
    compressor.prepare(spec);
    saturation.prepare(spec);
    output.prepare(spec);
}

void EasyRecAudioProcessor::releaseResources()
{
    eq.reset();
    compressor.reset();
    saturation.reset();
    output.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EasyRecAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Supporta solo stereo in e stereo out
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
}
#endif

void EasyRecAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Pulizia canali output extra
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // === Parametri (APVTS) ===
    const float lowCutHz   = *parameters.getRawParameterValue("lowCut");
    const float toneNorm   = *parameters.getRawParameterValue("tone");
    const float eqOnV      = *parameters.getRawParameterValue("eqOn");
    const float compAmt    = *parameters.getRawParameterValue("comp");
    const float compSoftV  = *parameters.getRawParameterValue("compSoft");
    const float satAmt     = *parameters.getRawParameterValue("satur");
    const float satSoftV   = *parameters.getRawParameterValue("satSoft");
    const float compOnV    = *parameters.getRawParameterValue("compOn");
    const float satOnV     = *parameters.getRawParameterValue("satOn");
    const float outNorm    = *parameters.getRawParameterValue("out");

    // Mapping: tone 0..1 -> -10..+10 dB
    const float toneDb = juce::jmap(toneNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const bool outIsMute = (outNorm <= 0.0001f);
    const auto normToDb = [](float norm) -> float
    {
        constexpr float seg = 1.0f / 9.0f; // 9 segmenti uguali
        if (norm <= seg * 1.0f) return juce::jmap(norm, seg * 0.0f, seg * 1.0f, -100.0f, -40.0f);
        if (norm <= seg * 2.0f) return juce::jmap(norm, seg * 1.0f, seg * 2.0f,  -40.0f, -30.0f);
        if (norm <= seg * 3.0f) return juce::jmap(norm, seg * 2.0f, seg * 3.0f,  -30.0f, -20.0f);
        if (norm <= seg * 4.0f) return juce::jmap(norm, seg * 3.0f, seg * 4.0f,  -20.0f, -15.0f);
        if (norm <= seg * 5.0f) return juce::jmap(norm, seg * 4.0f, seg * 5.0f,  -15.0f, -10.0f);
        if (norm <= seg * 6.0f) return juce::jmap(norm, seg * 5.0f, seg * 6.0f,  -10.0f,   0.0f);
        if (norm <= seg * 7.0f) return juce::jmap(norm, seg * 6.0f, seg * 7.0f,    0.0f,   3.3333f);
        if (norm <= seg * 8.0f) return juce::jmap(norm, seg * 7.0f, seg * 8.0f,    3.3333f, 6.6667f);
        return juce::jmap(norm, seg * 8.0f, seg * 9.0f,    6.6667f, 10.0f);
    };
    const float outDb = normToDb(outNorm);

    const bool eqOn = (eqOnV >= 0.5f);
    if (eqOn)
    {
        eq.setLowCutFreq(lowCutHz);
        eq.setToneAmount(toneDb);
    }
    const bool compOn = (compOnV >= 0.5f);
    const bool satOn = (satOnV >= 0.5f);

    compressor.setSoftMode(compSoftV >= 0.5f);
    compressor.setAmount(compAmt);
    saturation.setSoftMode(satSoftV >= 0.5f);
    saturation.setAmount(satAmt);
    if (outIsMute)
        output.setGainDb(-100.0f);
    else
        output.setGainDb(outDb);

    // Elaborazione a catena
    if (eqOn)
        eq.processBlock(buffer);
    if (compOn)
        compressor.processBlock(buffer);
    if (satOn)
        saturation.processBlock(buffer);
    output.processBlock(buffer);
}

//==============================================================================
void EasyRecAudioProcessor::updateEQFilters(float lowCutFreq, float toneAmount)
{
    eq.setLowCutFreq(lowCutFreq);
    eq.setToneAmount(toneAmount);
}


void EasyRecAudioProcessor::setCompressorAmount(float amount)
{
    compressor.setAmount(amount);
}

void EasyRecAudioProcessor::setCompressorSoftMode(bool soft)
{
    compressor.setSoftMode(soft);
}

void EasyRecAudioProcessor::setSaturationAmount(float amount)
{
    saturation.setAmount(amount);
}

void EasyRecAudioProcessor::setSaturationSoftMode(bool soft)
{
    saturation.setSoftMode(soft);
}

void EasyRecAudioProcessor::setOutputGainDb(float gainDb)
{
    output.setGainDb(gainDb);
}

//==============================================================================
bool EasyRecAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* EasyRecAudioProcessor::createEditor()
{
    return new EasyRecAudioProcessorEditor (*this);
}

//==============================================================================
void EasyRecAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    parameters.state.writeToStream(stream);
}

void EasyRecAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
        parameters.replaceState(tree);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EasyRecAudioProcessor();
}

EasyRecAudioProcessor::APVTS::ParameterLayout EasyRecAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "lowCut", 1 }, "Low Cut",
        juce::NormalisableRange<float>(20.0f, 200.0f, 0.1f),
        110.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "tone", 1 }, "Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "eqOn", 1 }, "EQ On",
        true));


    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp", 1 }, "Compressor",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "compSoft", 1 }, "Comp Soft Mode",
        true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "satur", 1 }, "Saturation",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "satSoft", 1 }, "Saturation Soft Mode",
        true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "compOn", 1 }, "Comp On",
        true));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "satOn", 1 }, "Saturation On",
        true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "out", 1 }, "Output",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        6.0f / 9.0f));

    return { params.begin(), params.end() };
}

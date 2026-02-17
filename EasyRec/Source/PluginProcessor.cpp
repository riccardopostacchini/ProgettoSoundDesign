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

    deEsserBandL.prepare(spec);
    deEsserBandR.prepare(spec);
    deEsserBandL.reset();
    deEsserBandR.reset();
    deEsserBandL.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 7000.0f, 2.2f);
    deEsserBandR.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, 7000.0f, 2.2f);

    deEssEnv = 0.0f;
    deEssGainSmoothed = 1.0f;
    deEssAttackCoeff = std::exp(-1.0f / (0.001f * 2.0f * (float) sampleRate));
    deEssReleaseCoeff = std::exp(-1.0f / (0.001f * 80.0f * (float) sampleRate));
    deEssGainAttackCoeff = std::exp(-1.0f / (0.001f * 2.0f * (float) sampleRate));
    deEssGainReleaseCoeff = std::exp(-1.0f / (0.001f * 60.0f * (float) sampleRate));
}

void EasyRecAudioProcessor::releaseResources()
{
    eq.reset();
    compressor.reset();
    saturation.reset();
    output.reset();
    deEsserBandL.reset();
    deEsserBandR.reset();
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
    const float eqBassNorm = *parameters.getRawParameterValue("lowCut");
    const float inputNorm  = *parameters.getRawParameterValue("tone");
    const float compAmtNorm = *parameters.getRawParameterValue("comp");
    const float compSoftV  = *parameters.getRawParameterValue("compSoft");
    const float eqTrebleNorm = *parameters.getRawParameterValue("satur");
    const float outNorm    = *parameters.getRawParameterValue("out");

    const float inputDb = juce::jmap(inputNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const float compAmtDb = juce::jmap(compAmtNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const float eqBassDb = juce::jmap(eqBassNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const float eqTrebleDb = juce::jmap(eqTrebleNorm, 0.0f, 1.0f, -10.0f, 10.0f);
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

    compressor.setSoftMode(compSoftV >= 0.5f);
    compressor.setInputDriveDb(inputDb);
    compressor.setAmount(compAmtDb);

    eq.setSoftPreset(compSoftV >= 0.5f);
    eq.setBassAmount(eqBassDb);
    eq.setTrebleAmount(eqTrebleDb);

    // Saturazione analogica nascosta sempre leggera.
    saturation.setSoftMode(true);
    saturation.setAmount(0.12f);

    if (outIsMute)
        output.setGainDb(-100.0f);
    else
        output.setGainDb(outDb);

    // Nuova catena: Input -> Comp -> EQ -> De-esser interno -> Saturazione interna -> Output
    compressor.processBlock(buffer);
    eq.processBlock(buffer);
    processHiddenDeEsser(buffer);
    saturation.processBlock(buffer);

    output.processBlock(buffer);
}

//==============================================================================
void EasyRecAudioProcessor::updateEQFilters(float lowCutFreq, float toneAmount)
{
    eq.setBassAmount(lowCutFreq);
    eq.setTrebleAmount(toneAmount);
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

void EasyRecAudioProcessor::processHiddenDeEsser(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples == 0 || numChannels == 0)
        return;

    float* left = buffer.getWritePointer(0);
    float* right = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    constexpr float thresholdDb = -26.0f;
    constexpr float ratioOver = 2.2f;
    constexpr float maxReductionDb = 6.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = left[i];
        const float inR = right != nullptr ? right[i] : inL;

        const float bandL = deEsserBandL.processSample(inL);
        const float bandR = deEsserBandR.processSample(inR);
        const float sib = juce::jmax(std::abs(bandL), std::abs(bandR));

        const float coeffEnv = (sib > deEssEnv) ? deEssAttackCoeff : deEssReleaseCoeff;
        deEssEnv = coeffEnv * deEssEnv + (1.0f - coeffEnv) * sib;

        const float envDb = juce::Decibels::gainToDecibels(deEssEnv + 1.0e-9f);
        float targetReductionDb = 0.0f;
        if (envDb > thresholdDb)
            targetReductionDb = juce::jmin(maxReductionDb, (envDb - thresholdDb) * (1.0f - (1.0f / ratioOver)));

        const float targetGain = juce::Decibels::decibelsToGain(-targetReductionDb);
        const float coeffGain = (targetGain < deEssGainSmoothed) ? deEssGainAttackCoeff : deEssGainReleaseCoeff;
        deEssGainSmoothed = coeffGain * deEssGainSmoothed + (1.0f - coeffGain) * targetGain;

        left[i] *= deEssGainSmoothed;
        if (right != nullptr)
            right[i] *= deEssGainSmoothed;
    }
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
        juce::ParameterID { "lowCut", 1 }, "EQ Bass",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "tone", 1 }, "Input",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "eqOn", 1 }, "EQ On",
        true));


    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "comp", 1 }, "Comp Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "compSoft", 1 }, "Comp Soft Mode",
        true));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "satur", 1 }, "EQ Treble",
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

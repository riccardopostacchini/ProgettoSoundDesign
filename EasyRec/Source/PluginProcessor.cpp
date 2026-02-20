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
    currentSampleRate = sampleRate;
    const float nyquist = juce::jmax(100.0f, 0.5f * (float) sampleRate);
    const auto clampFreq = [nyquist](float hz)
    {
        return juce::jlimit(10.0f, nyquist - 1.0f, hz);
    };

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    eq.prepare(spec);
    compressor.prepare(spec);
    saturation.prepare(spec);
    output.prepare(spec);
    roomReverb.reset();
    churchReverb.reset();
    slapDelayL.prepare(spec);
    slapDelayR.prepare(spec);
    eighthDelayL.prepare(spec);
    eighthDelayR.prepare(spec);
    slapDelayL.reset();
    slapDelayR.reset();
    eighthDelayL.reset();
    eighthDelayR.reset();

    deEsserBandL.prepare(spec);
    deEsserBandR.prepare(spec);
    deEsserBandL.reset();
    deEsserBandR.reset();
    deEsserBandL.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, clampFreq(7000.0f), 2.2f);
    deEsserBandR.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, clampFreq(7000.0f), 2.2f);

    deEssEnv = 0.0f;
    deEssGainSmoothed = 1.0f;
    deEssAttackCoeff = std::exp(-1.0f / (0.001f * 2.0f * (float) sampleRate));
    deEssReleaseCoeff = std::exp(-1.0f / (0.001f * 80.0f * (float) sampleRate));
    deEssGainAttackCoeff = std::exp(-1.0f / (0.001f * 2.0f * (float) sampleRate));
    deEssGainReleaseCoeff = std::exp(-1.0f / (0.001f * 60.0f * (float) sampleRate));

    safetyHpL.prepare(spec);
    safetyHpR.prepare(spec);
    safetyLpL.prepare(spec);
    safetyLpR.prepare(spec);
    safetyHpL.reset();
    safetyHpR.reset();
    safetyLpL.reset();
    safetyLpR.reset();
    // Hidden profile: aggressive pop
    safetyHpL.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, clampFreq(45.0f), 0.72f);
    safetyHpR.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, clampFreq(45.0f), 0.72f);
    safetyLpL.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, clampFreq(17000.0f), 0.72f);
    safetyLpR.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, clampFreq(17000.0f), 0.72f);

    hiddenLimiterGain = 1.0f;
    hiddenLimiterAttackCoeff = std::exp(-1.0f / (0.001f * 0.3f * (float) sampleRate));
    hiddenLimiterReleaseCoeff = std::exp(-1.0f / (0.001f * 25.0f * (float) sampleRate));
}

void EasyRecAudioProcessor::releaseResources()
{
    eq.reset();
    compressor.reset();
    saturation.reset();
    output.reset();
    roomReverb.reset();
    churchReverb.reset();
    slapDelayL.reset();
    slapDelayR.reset();
    eighthDelayL.reset();
    eighthDelayR.reset();
    deEsserBandL.reset();
    deEsserBandR.reset();
    safetyHpL.reset();
    safetyHpR.reset();
    safetyLpL.reset();
    safetyLpR.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EasyRecAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();
    const auto mainIn = layouts.getMainInputChannelSet();

    // Supporta mono e stereo
    if (mainOut != juce::AudioChannelSet::mono()
        && mainOut != juce::AudioChannelSet::stereo())
        return false;

#if ! JucePlugin_IsSynth
    if (mainIn != mainOut)
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

    // Input meter (pre-chain)
    {
        float peak = 0.0f;
        const int channels = juce::jmin(buffer.getNumChannels(), juce::jmax(1, totalNumInputChannels));
        const int samples = buffer.getNumSamples();
        for (int ch = 0; ch < channels; ++ch)
        {
            const float* read = buffer.getReadPointer(ch);
            for (int i = 0; i < samples; ++i)
                peak = juce::jmax(peak, std::abs(read[i]));
        }

        float peakDb = juce::Decibels::gainToDecibels(peak, -60.0f);
        peakDb = juce::jlimit(-60.0f, 6.0f, peakDb);
        const float previousDb = inputMeterDb.load();
        constexpr float meterReleaseDbPerBlock = 0.8f;
        const float smoothedDb = (peakDb > previousDb) ? peakDb : juce::jmax(peakDb, previousDb - meterReleaseDbPerBlock);
        inputMeterDb.store(smoothedDb);
    }

    // === Parametri (APVTS) ===
    const float eqBassNorm = *parameters.getRawParameterValue("lowCut");
    const float inputNorm  = *parameters.getRawParameterValue("tone");
    const float compAmtNorm = *parameters.getRawParameterValue("comp");
    const float compSoftV  = *parameters.getRawParameterValue("compSoft");
    const float eqTrebleNorm = *parameters.getRawParameterValue("satur");
    const float eqOnV = *parameters.getRawParameterValue("eqOn");
    const float compOnV = *parameters.getRawParameterValue("compOn");
    const float satOnV = *parameters.getRawParameterValue("satOn");
    const float outNorm    = *parameters.getRawParameterValue("out");
    const float roomNorm = *parameters.getRawParameterValue("room");
    const float churchNorm = *parameters.getRawParameterValue("church");
    const float slapNorm = *parameters.getRawParameterValue("slap");
    const float eighthNorm = *parameters.getRawParameterValue("eighth");
    const bool roomOn = (*parameters.getRawParameterValue("roomOn") >= 0.5f);
    const bool churchOn = (*parameters.getRawParameterValue("churchOn") >= 0.5f);
    const bool slapOn = (*parameters.getRawParameterValue("slapOn") >= 0.5f);
    const bool eighthOn = (*parameters.getRawParameterValue("eighthOn") >= 0.5f);

    const float inputDb = juce::jmap(inputNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const float compAmtDb = juce::jmap(compAmtNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const float eqBassDb = juce::jmap(eqBassNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const float eqTrebleDb = juce::jmap(eqTrebleNorm, 0.0f, 1.0f, -10.0f, 10.0f);
    const bool lowOn = (eqOnV >= 0.5f);
    const bool compOn = (compOnV >= 0.5f);
    const bool trebleOn = (satOnV >= 0.5f);
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
    const auto sliderNormToSend = [](float paramNorm) -> float
    {
        const float dbValue = juce::jmap(paramNorm, 0.0f, 1.0f, -10.0f, 10.0f);
        const float send = juce::jmap(dbValue, -10.0f, 10.0f, 0.0f, 1.0f);
        return juce::jlimit(0.0f, 1.0f, std::pow(send, 1.2f));
    };
    const float roomSend = roomOn ? sliderNormToSend(roomNorm) : 0.0f;
    const float churchSend = churchOn ? sliderNormToSend(churchNorm) : 0.0f;
    const float slapSend = slapOn ? sliderNormToSend(slapNorm) : 0.0f;
    const float eighthSend = eighthOn ? sliderNormToSend(eighthNorm) : 0.0f;

    compressor.setSoftMode(compSoftV >= 0.5f);
    compressor.setInputDriveDb(inputDb);
    compressor.setAmount(compAmtDb);

    eq.setSoftPreset(compSoftV >= 0.5f);
    eq.setBassAmount(lowOn ? eqBassDb : 0.0f);
    eq.setTrebleAmount(trebleOn ? eqTrebleDb : 0.0f);

    // Saturazione analogica nascosta (profilo pop: piu' evidente).
    saturation.setSoftMode(true);
    saturation.setAmount(0.18f);

    if (outIsMute)
        output.setGainDb(-100.0f);
    else
        output.setGainDb(outDb);

    // Nuova catena: Input -> Comp -> EQ -> De-esser interno -> Saturazione interna -> Output
    if (compOn)
        compressor.processBlock(buffer);

    if (lowOn || trebleOn)
        eq.processBlock(buffer);

    processHiddenDeEsser(buffer);
    saturation.processBlock(buffer);
    processHiddenSafetyFilters(buffer);
    processHiddenPeakProtector(buffer);

    // === Screen 2 FX (parallel sends): Room, Church, Slap, Eighth ===
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples > 0 && numChannels > 0)
    {
        juce::AudioBuffer<float> dry(buffer);
        juce::AudioBuffer<float> fxBuffer(numChannels, numSamples);
        fxBuffer.clear();

        // Room reverb (short/medium ambience)
        if (roomSend > 0.0001f)
        {
            juce::AudioBuffer<float> roomIn(dry);
            roomIn.applyGain(roomSend);

            juce::dsp::Reverb::Parameters roomParams;
            roomParams.roomSize = 0.42f;
            roomParams.damping = 0.48f;
            roomParams.width = 1.0f;
            roomParams.freezeMode = 0.0f;
            roomParams.wetLevel = 0.30f;
            roomParams.dryLevel = 0.0f;
            roomReverb.setParameters(roomParams);

            juce::dsp::AudioBlock<float> roomBlock(roomIn);
            juce::dsp::ProcessContextReplacing<float> roomContext(roomBlock);
            roomReverb.process(roomContext);
            for (int ch = 0; ch < numChannels; ++ch)
                fxBuffer.addFrom(ch, 0, roomIn, ch, 0, numSamples);
        }

        // Church reverb (longer tail)
        if (churchSend > 0.0001f)
        {
            juce::AudioBuffer<float> churchIn(dry);
            churchIn.applyGain(churchSend);

            juce::dsp::Reverb::Parameters churchParams;
            churchParams.roomSize = 0.86f;
            churchParams.damping = 0.35f;
            churchParams.width = 1.0f;
            churchParams.freezeMode = 0.0f;
            churchParams.wetLevel = 0.38f;
            churchParams.dryLevel = 0.0f;
            churchReverb.setParameters(churchParams);

            juce::dsp::AudioBlock<float> churchBlock(churchIn);
            juce::dsp::ProcessContextReplacing<float> churchContext(churchBlock);
            churchReverb.process(churchContext);
            for (int ch = 0; ch < numChannels; ++ch)
                fxBuffer.addFrom(ch, 0, churchIn, ch, 0, numSamples);
        }

        // Delay times
        const double bpm = [this]() -> double
        {
            if (auto* head = getPlayHead())
            {
                if (auto pos = head->getPosition())
                {
                    if (auto bpm = pos->getBpm(); bpm.hasValue() && *bpm > 1.0)
                        return *bpm;
                }
            }
            return 120.0;
        }();

        const int slapDelaySamples = (int) juce::jlimit(1.0, 2000.0, currentSampleRate * 0.11); // ~110ms
        const int eighthDelaySamples = (int) juce::jlimit(1.0, 4000.0, currentSampleRate * (30.0 / bpm)); // 1/8 note

        for (int i = 0; i < numSamples; ++i)
        {
            const float inL = dry.getSample(0, i);
            const float inR = (numChannels > 1) ? dry.getSample(1, i) : inL;

            // Slap delay (low feedback)
            slapDelayL.setDelay((float) slapDelaySamples);
            slapDelayR.setDelay((float) slapDelaySamples);
            const float slapReadL = slapDelayL.popSample(0);
            const float slapReadR = slapDelayR.popSample(0);
            slapDelayL.pushSample(0, inL * slapSend + slapReadL * 0.18f);
            slapDelayR.pushSample(0, inR * slapSend + slapReadR * 0.18f);
            fxBuffer.addSample(0, i, slapReadL * 0.55f);
            if (numChannels > 1)
                fxBuffer.addSample(1, i, slapReadR * 0.55f);

            // Eighth delay (moderate feedback, ping-pong feel)
            eighthDelayL.setDelay((float) eighthDelaySamples);
            eighthDelayR.setDelay((float) eighthDelaySamples);
            const float eighthReadL = eighthDelayL.popSample(0);
            const float eighthReadR = eighthDelayR.popSample(0);
            const float pingL = inL * eighthSend + eighthReadR * 0.33f;
            const float pingR = inR * eighthSend + eighthReadL * 0.33f;
            eighthDelayL.pushSample(0, pingL);
            eighthDelayR.pushSample(0, pingR);
            fxBuffer.addSample(0, i, eighthReadL * 0.5f);
            if (numChannels > 1)
                fxBuffer.addSample(1, i, eighthReadR * 0.5f);
        }

        // Sum parallel FX with safety headroom.
        buffer.addFrom(0, 0, fxBuffer, 0, 0, numSamples, 0.7f);
        if (numChannels > 1)
            buffer.addFrom(1, 0, fxBuffer, 1, 0, numSamples, 0.7f);
    }

    output.processBlock(buffer);

    // Output meter (post-chain).
    float peak = 0.0f;
    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    for (int ch = 0; ch < channels; ++ch)
    {
        const float* read = buffer.getReadPointer(ch);
        for (int i = 0; i < samples; ++i)
            peak = juce::jmax(peak, std::abs(read[i]));
    }

    float peakDb = juce::Decibels::gainToDecibels(peak, -60.0f);
    peakDb = juce::jlimit(-60.0f, 6.0f, peakDb);

    const float previousDb = outputMeterDb.load();
    constexpr float meterReleaseDbPerBlock = 0.8f;
    const float smoothedDb = (peakDb > previousDb) ? peakDb : juce::jmax(peakDb, previousDb - meterReleaseDbPerBlock);
    outputMeterDb.store(smoothedDb);
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

    constexpr float thresholdDb = -30.0f;
    constexpr float ratioOver = 3.0f;
    constexpr float maxReductionDb = 8.0f;

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

void EasyRecAudioProcessor::processHiddenSafetyFilters(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    float* left = buffer.getWritePointer(0);
    float* right = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        left[i] = safetyLpL.processSample(safetyHpL.processSample(left[i]));
        if (right != nullptr)
            right[i] = safetyLpR.processSample(safetyHpR.processSample(right[i]));
    }
}

void EasyRecAudioProcessor::processHiddenPeakProtector(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples == 0)
        return;

    float* left = buffer.getWritePointer(0);
    float* right = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1) : nullptr;

    const float limiterThreshold = juce::Decibels::decibelsToGain(-2.0f); // ~ -2 dBFS
    const float satDrive = 1.15f;
    const float satNorm = std::tanh(satDrive);

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = left[i];
        const float inR = right != nullptr ? right[i] : inL;
        const float peak = juce::jmax(std::abs(inL), std::abs(inR));

        float targetGain = 1.0f;
        if (peak > limiterThreshold)
            targetGain = limiterThreshold / juce::jmax(peak, 1.0e-9f);

        const float coeff = (targetGain < hiddenLimiterGain) ? hiddenLimiterAttackCoeff : hiddenLimiterReleaseCoeff;
        hiddenLimiterGain = coeff * hiddenLimiterGain + (1.0f - coeff) * targetGain;

        float outL = inL * hiddenLimiterGain;
        float outR = inR * hiddenLimiterGain;

        // Soft clip molto leggero per trattenere i transienti finali.
        outL = std::tanh(outL * satDrive) / satNorm;
        outR = std::tanh(outR * satDrive) / satNorm;

        left[i] = outL;
        if (right != nullptr)
            right[i] = outR;
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

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "room", 1 }, "Room Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "church", 1 }, "Church Reverb",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "slap", 1 }, "Slap Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "eighth", 1 }, "Eighth Delay",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "roomOn", 1 }, "Room On",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "churchOn", 1 }, "Church On",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "slapOn", 1 }, "Slap On",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "eighthOn", 1 }, "Eighth On",
        false));

    return { params.begin(), params.end() };
}

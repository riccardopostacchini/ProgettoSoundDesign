#include "CompressorModule.h"

CompressorModule::CompressorModule() {}

CompressorModule::~CompressorModule() {}

void CompressorModule::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = (float) spec.sampleRate;
    setSoftMode(true);
    gainReductionDbSmoothed = 0.0f;
}

void CompressorModule::updateTimeConstants()
{
    const auto safeAttack = juce::jmax(0.1f, attackMs);
    const auto safeRelease = juce::jmax(0.1f, releaseMs);

    attackCoeff = std::exp(-1.0f / (0.001f * safeAttack * sampleRate));
    releaseCoeff = std::exp(-1.0f / (0.001f * safeRelease * sampleRate));
}

float CompressorModule::computeGainReductionDb(float levelDb) const
{
    const float overDb = levelDb - thresholdDb;
    const float slope = 1.0f - (1.0f / ratio);

    if (kneeDb <= 0.0f)
    {
        if (overDb <= 0.0f)
            return 0.0f;

        return slope * overDb;
    }

    const float halfKnee = 0.5f * kneeDb;

    if (overDb <= -halfKnee)
        return 0.0f;

    if (overDb >= halfKnee)
        return slope * overDb;

    const float x = overDb + halfKnee;
    return slope * (x * x) / (2.0f * kneeDb);
}

void CompressorModule::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples == 0 || numChannels == 0)
        return;

    float* left = buffer.getWritePointer(0);
    float* right = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    const float inDrive = juce::Decibels::decibelsToGain(inputDriveDb);
    const float inDriveInv = 1.0f / juce::jmax(0.001f, inDrive);

    const float amountScale = juce::jmap(amountDb, -10.0f, 10.0f, 0.0f, 2.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        float inL = left[i] * inDrive;
        float inR = right != nullptr ? right[i] * inDrive : inL;

        const float sidechain = juce::jmax(std::abs(inL), std::abs(inR));
        const float levelDb = juce::Decibels::gainToDecibels(sidechain + 1.0e-9f);

        const float reductionDb = computeGainReductionDb(levelDb) * amountScale;

        const float coeff = (reductionDb > gainReductionDbSmoothed) ? attackCoeff : releaseCoeff;
        gainReductionDbSmoothed = coeff * gainReductionDbSmoothed + (1.0f - coeff) * reductionDb;

        const float compGain = juce::Decibels::decibelsToGain(-gainReductionDbSmoothed);
        float outL = inL * compGain;
        float outR = inR * compGain;

        // Micro-saturazione analogica sui transienti compressi.
        const float satDrive = softMode ? 1.12f : 1.22f;
        const float satNorm = std::tanh(satDrive);
        outL = std::tanh(outL * satDrive) / satNorm;
        outR = std::tanh(outR * satDrive) / satNorm;

        // Compensa input drive: controlla pressione, non volume finale.
        left[i] = outL * inDriveInv;
        if (right != nullptr)
            right[i] = outR * inDriveInv;
    }
}

void CompressorModule::setAmount(float newAmountDb)
{
    amountDb = juce::jlimit(-10.0f, 10.0f, newAmountDb);
}

void CompressorModule::setInputDriveDb(float inputDb)
{
    inputDriveDb = juce::jlimit(-10.0f, 10.0f, inputDb);
}

void CompressorModule::setSoftMode(bool soft)
{
    softMode = soft;

    if (softMode)
    {
        ratio = 4.0f;
        attackMs = 5.0f;
        releaseMs = 50.0f;
        kneeDb = 8.0f;
        thresholdDb = -24.0f;
    }
    else
    {
        ratio = 10.0f;
        attackMs = 1.0f;
        releaseMs = 20.0f;
        kneeDb = 0.0f;
        thresholdDb = -20.0f;
    }

    updateTimeConstants();
}

void CompressorModule::reset()
{
    gainReductionDbSmoothed = 0.0f;
}

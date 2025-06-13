#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct KnoblookAndFeel : public juce::LookAndFeel_V4
{
    juce::Drawable* knobImage = nullptr;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        if (knobImage != nullptr)
        {
            const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
            g.addTransform(juce::AffineTransform::rotation(angle, x + width / 2.0f, y + height / 2.0f));
            knobImage->drawWithin(g, juce::Rectangle<float>(x, y, width, height), juce::RectanglePlacement::centred, 1.0f);
        }
        else
        {
            juce::LookAndFeel_V4::drawRotarySlider(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, slider);
        }
    }
};

class EasyRecAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    EasyRecAudioProcessorEditor (EasyRecAudioProcessor&);
    ~EasyRecAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    EasyRecAudioProcessor& audioProcessor;

    // Background
    juce::Image backgroundImage;

    // Knobs
    juce::Slider compKnob;
    KnoblookAndFeel compKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> compKnobDrawable;

    juce::Slider lowKnob;
    KnoblookAndFeel lowKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> lowKnobDrawable;

    juce::Slider toneKnob;
    KnoblookAndFeel toneKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> toneKnobDrawable;

    juce::Slider deeKnob;
    KnoblookAndFeel deeKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> deeKnobDrawable;

    juce::Slider satKnob;
    KnoblookAndFeel satKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> satKnobDrawable;

    juce::Slider outKnob;
    KnoblookAndFeel outKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> outKnobDrawable;

    // Toggle Soft/Hard Comp
    juce::DrawableButton toggleCompButton { "ToggleComp", juce::DrawableButton::ImageRaw };
    std::unique_ptr<juce::Drawable> softHighlightDrawable;
    std::unique_ptr<juce::Drawable> hardHighlightDrawable;
    bool isSoftMode = true;
    juce::Rectangle<int> softHighlightArea;
    juce::Rectangle<int> hardHighlightArea;

    // Toggle Soft/Hard Satur
    bool isSoftSaturMode = true;
    juce::TextButton saturToggleButton;
    juce::Rectangle<int> softSatHighlightArea;
    juce::Rectangle<int> hardSatHighlightArea;
    std::unique_ptr<juce::Drawable> softSatHighlightDrawable;
    std::unique_ptr<juce::Drawable> hardSatHighlightDrawable;

    // Animated rectangles for transitions
    juce::Rectangle<float> currentCompHighlightRect;
    juce::Rectangle<float> currentSaturHighlightRect;

    // Animation
    void timerCallback() override;
    bool compAnimating = false;
    bool saturAnimating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyRecAudioProcessorEditor)
};


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

class ResettableSlider : public juce::Slider
{
public:
    ResettableSlider(float defaultVal) : defaultValue(defaultVal) {}

    void mouseDoubleClick(const juce::MouseEvent&) override
    {
        setValue(defaultValue, juce::sendNotificationSync);
    }

private:
    float defaultValue;
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

    //COMP KNOB
    ResettableSlider compKnob { 0.5f };
    KnoblookAndFeel compKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> compKnobDrawable;
    juce::Label compLabel;

    
    //EQ KNOB
    ResettableSlider lowKnob { 0.5f };
    KnoblookAndFeel lowKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> lowKnobDrawable;
    juce::Label lowLabel;

    ResettableSlider toneKnob { 0.5f };
    KnoblookAndFeel toneKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> toneKnobDrawable;
    juce::Label toneLabel;

    //DE-ESSER KNOB
    ResettableSlider deeKnob { 0.5f };
    KnoblookAndFeel deeKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> deeKnobDrawable;
    juce::Label deeLabel;

    //SATURATOR KNOB
    ResettableSlider satKnob { 0.5f };
    KnoblookAndFeel satKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> satKnobDrawable;
    juce::Label satLabel;

    //OUTPUT KNOB
    ResettableSlider outKnob { 0.5f };
    KnoblookAndFeel outKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> outKnobDrawable;
    juce::Label outLabel;

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

    // FONT
    juce::Typeface::Ptr earlyGameBoyFont;
    
    // Animated rectangles for transitions
    juce::Rectangle<float> currentCompHighlightRect;
    juce::Rectangle<float> currentSaturHighlightRect;

    // Animation
    void timerCallback() override;
    bool compAnimating = false;
    bool saturAnimating = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyRecAudioProcessorEditor)
};


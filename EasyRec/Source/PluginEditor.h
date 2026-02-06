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

    using APVTS = EasyRecAudioProcessor::APVTS;

    // Background (solo interfaccia B)
    juce::Image backgroundImage;

    //COMP SLIDER
    ResettableSlider compKnob { 0.5f };
    juce::Label compLabel;

    
    //EQ KNOB
    ResettableSlider lowKnob { 110.0f };
    KnoblookAndFeel lowKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> lowKnobDrawable;
    juce::Label lowLabelDescription;
    juce::Label lowLabelValue;

    ResettableSlider toneKnob { 0.5f };
    KnoblookAndFeel toneKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> toneKnobDrawable;
    juce::Label toneLabelDescription;
    juce::Label toneLabelValue;


    //SATURATOR SLIDER
    ResettableSlider satKnob { 0.5f };
    juce::Label satLabel;

    //OUTPUT KNOB
    ResettableSlider outKnob { 6.0f / 9.0f };
    KnoblookAndFeel outKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> outKnobDrawable;
    juce::Label outLabel;
    juce::Image buttonSliderImage;
    juce::Rectangle<int> satSliderBaseRect;
    juce::Rectangle<int> compSliderBaseRect;
    juce::Image eqOnImage;
    juce::Rectangle<int> eqOnRect;

    // On/Off buttons (B)
    juce::TextButton satOnOffButton;
    juce::TextButton compOnOffButton;
    juce::TextButton eqOnOffButton;
    juce::TextButton animOnOffButton;
    bool animationsEnabled = true;

    // Toggle Soft/Hard Comp
    juce::DrawableButton toggleCompButton { "ToggleComp", juce::DrawableButton::ImageRaw };
    bool isSoftMode = true;

    // Toggle Soft/Hard Satur
    bool isSoftSaturMode = true;
    juce::TextButton saturToggleButton;
    std::unique_ptr<juce::Drawable> compBDrawable;
    std::unique_ptr<juce::Drawable> satBDrawable;
    std::unique_ptr<juce::Drawable> compADrawable;
    std::unique_ptr<juce::Drawable> satADrawable;
    juce::Rectangle<int> compBRect;
    juce::Rectangle<int> satBRect;

    // FONT
    juce::Typeface::Ptr earlyGameBoyFont;
    
    // Animation
    void timerCallback() override;

    void updateEQ();

    // Micro-movimento stile Game Boy (interfaccia B)
    float spritePhase = 0.0f;
    float spriteSpeed = 0.08f;
    int spriteAmplitudePx = 2;

    // APVTS attachments
    std::unique_ptr<APVTS::SliderAttachment> compAttachment;
    std::unique_ptr<APVTS::SliderAttachment> lowAttachment;
    std::unique_ptr<APVTS::SliderAttachment> toneAttachment;
    std::unique_ptr<APVTS::SliderAttachment> satAttachment;
    std::unique_ptr<APVTS::SliderAttachment> outAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> compSoftAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> satSoftAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> compOnAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> satOnAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> eqOnAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyRecAudioProcessorEditor)
};

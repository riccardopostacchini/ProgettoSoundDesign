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

    // Background
    juce::Image backgroundImage;
    juce::Image backgroundImageB;
    juce::Image catImage;
    juce::Rectangle<int> catRect;
    bool isScreenB = false;
    juce::TextButton screenToggleButton;
    juce::TextButton screenToggleButtonLeft;
    juce::TextButton screenBackButton;
    ResettableSlider roomKnob { 0.5f };
    ResettableSlider churchKnob { 0.5f };
    ResettableSlider slapKnob { 0.5f };
    ResettableSlider eighthKnob { 0.5f };
    juce::Label roomLabel;
    juce::Label churchLabel;
    juce::Label slapLabel;
    juce::Label eighthLabel;
    juce::TextButton roomOnOffButton;
    juce::TextButton churchOnOffButton;
    juce::TextButton slapOnOffButton;
    juce::TextButton eighthOnOffButton;

    //COMP SLIDER
    ResettableSlider compKnob { 0.5f };
    juce::Label compLabel;

    
    //EQ KNOB
    ResettableSlider bassKnob { 0.5f };
    KnoblookAndFeel bassKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> bassKnobDrawable;
    juce::Label bassLabelDescription;
    juce::Label bassLabelValue;

    ResettableSlider inputKnob { 0.5f };
    KnoblookAndFeel inputKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> inputKnobDrawable;
    juce::Label inputLabelDescription;
    juce::Label inputLabelValue;


    //TREBLE SLIDER
    ResettableSlider trebleKnob { 0.5f };
    juce::Label trebleLabel;

    //OUTPUT KNOB
    ResettableSlider outKnob { 6.0f / 9.0f };
    KnoblookAndFeel outKnobLookAndFeel;
    std::unique_ptr<juce::Drawable> outKnobDrawable;
    juce::Label outLabel;
    juce::Image buttonSliderImage;
    juce::Rectangle<int> trebleSliderBaseRect;
    juce::Rectangle<int> compSliderBaseRect;
    juce::Image eqOnImage;
    juce::Rectangle<int> eqOnRect;
    juce::Image introImage;
    std::unique_ptr<juce::Drawable> introGamevoiceDrawable;
    std::unique_ptr<juce::Drawable> introNomiDrawable;
    bool introActive = true;
    float introProgress = 0.0f; // 0..1
    int introDelayFrames = 90;  // 1.5s @60fps
    int introHoldFrames = 0;
    float introImageFade = 0.0f; // 0..1 (fade out)
    bool introPaused = false;

    // On/Off buttons (B)
    juce::TextButton trebleOnOffButton;
    juce::TextButton compOnOffButton;
    juce::TextButton eqOnOffButton;
    juce::TextButton animOnOffButton;
    bool animationsEnabled = true;
    juce::TextButton presetSwitchButton;
    juce::TextButton presetSwitchBackButton;
    juce::Label presetNameLabel;
    int currentPresetIndex = 0;
    bool presetDirty = false;

    struct PresetDefinition
    {
        const char* name;
        // input/output sono mantenuti per compatibilita' ma NON vengono applicati dai preset.
        // Ordine campi float: input, comp, eqBass, eqTreble, output
        float input, comp, eqBass, eqTreble, output;
        // Ordine bool: bassEqOn, compOn, trebleOn, compSoft, trebleSoft
        bool bassEqOn, compOn, trebleOn, compSoft, trebleSoft;
        // Ordine fx float: room, church, slap, eighth
        float room, church, slap, eighth;
        // Ordine fx bool: roomOn, churchOn, slapOn, eighthOn
        bool roomOn, churchOn, slapOn, eighthOn;
    };

    // Toggle Soft/Hard Comp
    juce::DrawableButton toggleCompButton { "ToggleComp", juce::DrawableButton::ImageRaw };
    bool isSoftMode = true;

    // Toggle Soft/Hard Treble
    bool isSoftTrebleMode = true;
    juce::TextButton trebleModeToggleButton;
    std::unique_ptr<juce::Drawable> compBDrawable;
    std::unique_ptr<juce::Drawable> trebleHardDrawable;
    std::unique_ptr<juce::Drawable> compADrawable;
    std::unique_ptr<juce::Drawable> trebleSoftDrawable;
    juce::Rectangle<int> compBRect;
    juce::Rectangle<int> trebleModeRect;

    // FONT
    juce::Typeface::Ptr earlyGameBoyFont;
    
    // Animation
    void timerCallback() override;

    void updateEQ();
    void applyPreset (int presetIndex);
    void updatePresetLabel();
    PresetDefinition getPresetValues (int presetIndex) const;
    bool isCurrentStateMatchingPreset (int presetIndex) const;

    // Micro-movimento stile Game Boy (interfaccia B)
    float spritePhase = 0.0f;
    float spriteSpeed = 0.08f;
    int spriteAmplitudePx = 2;

    // APVTS attachments
    std::unique_ptr<APVTS::SliderAttachment> compAttachment;
    std::unique_ptr<APVTS::SliderAttachment> bassAttachment;
    std::unique_ptr<APVTS::SliderAttachment> inputAttachment;
    std::unique_ptr<APVTS::SliderAttachment> trebleAttachment;
    std::unique_ptr<APVTS::SliderAttachment> outAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> compSoftAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> trebleSoftAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> compOnAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> trebleOnAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> eqOnAttachment;
    std::unique_ptr<APVTS::SliderAttachment> roomAttachment;
    std::unique_ptr<APVTS::SliderAttachment> churchAttachment;
    std::unique_ptr<APVTS::SliderAttachment> slapAttachment;
    std::unique_ptr<APVTS::SliderAttachment> eighthAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> roomOnAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> churchOnAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> slapOnAttachment;
    std::unique_ptr<APVTS::ButtonAttachment> eighthOnAttachment;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyRecAudioProcessorEditor)
};

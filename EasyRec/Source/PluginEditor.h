
#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

struct KnoblookAndFeel : public juce::LookAndFeel_V4
{
    juce::Drawable* knobImage = nullptr;
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
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

//==============================================================================
/**
*/
class EasyRecAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    EasyRecAudioProcessorEditor (EasyRecAudioProcessor&);
    ~EasyRecAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    EasyRecAudioProcessor& audioProcessor;

    // Sfondo
    juce::Image backgroundImage;
    
    // Controlli
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EasyRecAudioProcessorEditor)
};

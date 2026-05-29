#pragma once

#include <BaseProcessor.h>
#include <ParameterComponents.h>
#include <juce_gui_basics/juce_gui_basics.h>

class TB303LookAndFeel : public juce::LookAndFeel_V4
{
public:
    TB303LookAndFeel();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;
};

class TB303Editor final : public juce::AudioProcessorEditor
{
public:
    TB303Editor(mrta::BaseProcessor&);
    ~TB303Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;
    TB303LookAndFeel customLookAndFeel;

    mrta::ParameterSlider tuningSlider;
    mrta::ParameterSlider cutoffSlider;
    mrta::ParameterSlider resonanceSlider;
    mrta::ParameterSlider envelopeModSlider;
    mrta::ParameterSlider decaySlider;
    mrta::ParameterSlider accentSlider;
    mrta::ParameterSlider volumeSlider;
    juce::TextButton waveformButton;

    juce::Label tuningLabel;
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label envModLabel;
    juce::Label decayLabel;
    juce::Label accentLabel;
    juce::Label waveformLabel;
    juce::Label volumeLabel;

    void setupLabel(juce::Label& label);
    void updateWaveformButtonText();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TB303Editor)
};

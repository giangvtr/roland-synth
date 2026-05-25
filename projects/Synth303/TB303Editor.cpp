#include "TB303Editor.h"

// Width of the whole GUI
static constexpr int WIDTH { 250 };

// Height of each paramter knob on the paramEditor
static const int PARAM_HEIGHT { 100 };

// Silver and red color scheme
static const juce::Colour SILVER_BACKGROUND { juce::Colour(0xFFD3D3D3) };
static const juce::Colour SILVER_DARK { juce::Colour(0xFFA9A9A9) };
static const juce::Colour SILVER_LIGHT { juce::Colour(0xFFE8E8E8) };
static const juce::Colour RED_ACCENT { juce::Colour(0xFFCC0000) };
static const juce::Colour RED_ACCENT_LIGHT { juce::Colour(0xFFFF4444) };

TB303LookAndFeel::TB303LookAndFeel()
{
    // Set background colors to silver scheme
    setColour(juce::ResizableWindow::backgroundColourId, SILVER_BACKGROUND);
    
    // Slider colors with red accents
    setColour(juce::Slider::backgroundColourId, SILVER_DARK);
    setColour(juce::Slider::thumbColourId, RED_ACCENT);
    setColour(juce::Slider::trackColourId, SILVER_LIGHT);
    setColour(juce::Slider::textBoxOutlineColourId, RED_ACCENT);
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::black);
    setColour(juce::Slider::textBoxBackgroundColourId, SILVER_LIGHT);
    
    // Button/ComboBox colors with red accents
    setColour(juce::ComboBox::backgroundColourId, SILVER_LIGHT);
    setColour(juce::ComboBox::outlineColourId, RED_ACCENT);
    setColour(juce::ComboBox::textColourId, juce::Colours::black);
    setColour(juce::TextButton::buttonColourId, SILVER_DARK);
    setColour(juce::TextButton::textColourOnId, RED_ACCENT);
    setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    setColour(juce::TextButton::buttonOnColourId, RED_ACCENT);
    
    // Label and text colors
    setColour(juce::Label::textColourId, juce::Colours::black);
    setColour(juce::Label::backgroundColourId, SILVER_BACKGROUND);
    setColour(juce::Label::outlineColourId, SILVER_BACKGROUND);
}

TB303Editor::TB303Editor(mrta::BaseProcessor& p) :
    juce::AudioProcessorEditor(p),
    processor { p },
    paramEditor(processor.getParameterManager(), PARAM_HEIGHT)
{
    // Apply custom look and feel
    setLookAndFeel(&customLookAndFeel);
    
    addAndMakeVisible(paramEditor);

    // Calculate window height based on number of parameters
    const auto height { processor.getParameterManager().getParameters().size() * PARAM_HEIGHT };
    setSize(WIDTH, height);
}

TB303Editor::~TB303Editor()
{
    setLookAndFeel(nullptr);
}

void TB303Editor::paint(juce::Graphics& g)
{
    // Fill background with silver color
    g.fillAll(SILVER_BACKGROUND);
    
    // Add subtle border with red accent
    g.setColour(RED_ACCENT);
    g.drawRect(getLocalBounds(), 2);
}

void TB303Editor::resized()
{
    paramEditor.setBounds(getLocalBounds());
}

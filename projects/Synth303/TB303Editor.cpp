#include "TB303Editor.h"

// Width of the whole GUI
static constexpr int WIDTH { 250 };

// Height of each paramter knob on the paramEditor
static const int PARAM_HEIGHT { 100 };

TB303Editor::TB303Editor(mrta::BaseProcessor& p) :
    juce::AudioProcessorEditor(p),
    processor { p },
    paramEditor(processor.getParameterManager(), PARAM_HEIGHT)
{
    addAndMakeVisible(paramEditor);

    // Calculate window height based on number of parameters
    const auto height { processor.getParameterManager().getParameters().size() * PARAM_HEIGHT };
    setSize(WIDTH, height);
}

TB303Editor::~TB303Editor()
{
}

void TB303Editor::paint(juce::Graphics&)
{
}

void TB303Editor::resized()
{
    paramEditor.setBounds(getLocalBounds());
}

#include "RingModulatorEditor.h"

// Width of the whole GUI
static constexpr int WIDTH { 250 };

// Height of each paramter knob on the paramEditor
static const int PARAM_HEIGHT { 100 };

RingModulatorEditor::RingModulatorEditor(mrta::BaseProcessor& p) :
    juce::AudioProcessorEditor(p),
    processor { p },
    paramEditor(processor.getParameterManager(), PARAM_HEIGHT)
{
    addAndMakeVisible(paramEditor);

    // Calculate window height based on number of parameters
    const auto height { processor.getParameterManager().getParameters().size() * PARAM_HEIGHT };
    setSize(WIDTH, height);
}

RingModulatorEditor::~RingModulatorEditor()
{
}

void RingModulatorEditor::paint(juce::Graphics&)
{
}

void RingModulatorEditor::resized()
{
    paramEditor.setBounds(getLocalBounds());
}

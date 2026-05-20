#include "DelayEditor.h"

#include "DelayProcessor.h"

// Width of the whole GUI
static constexpr int WIDTH { 300 };

// Height of each paramter knob on the paramEditor
static const int PARAM_HEIGHT { 100 };

// Width of Meter component
static constexpr int METER_WIDTH { 25 };

DelayEditor::DelayEditor(mrta::BaseProcessor& p) :
    juce::AudioProcessorEditor(p),
    processor { p },
    paramEditor(processor.getParameterManager(), PARAM_HEIGHT),
    meterOutputComponent(static_cast<DelayProcessor&>(processor).getMeterOutput()),
    meterInputComponent(static_cast<DelayProcessor&>(processor).getMeterInput())
{
    addAndMakeVisible(paramEditor);
    addAndMakeVisible(meterOutputComponent);
    addAndMakeVisible(meterInputComponent);

    paramEditor.setLookAndFeel(&laf);

    // Calculate window height based on number of parameters
    const auto height { processor.getParameterManager().getParameters().size() * PARAM_HEIGHT };
    setSize(WIDTH, height);
}

DelayEditor::~DelayEditor()
{
    paramEditor.setLookAndFeel(nullptr);
}

void DelayEditor::paint(juce::Graphics&)
{
}

void DelayEditor::resized()
{
    juce::Rectangle<int> area = getLocalBounds();
    meterInputComponent.setBounds(area.removeFromLeft(METER_WIDTH));
    meterOutputComponent.setBounds(area.removeFromRight(METER_WIDTH));
    paramEditor.setBounds(area);
}

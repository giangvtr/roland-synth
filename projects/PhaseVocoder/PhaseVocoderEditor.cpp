#include "PhaseVocoderEditor.h"
#include "BaseProcessor.h"

// Width of the whole GUI
static constexpr int WIDTH { 250 };

// Height of each paramter knob on the paramEditor
static const int PARAM_HEIGHT { 100 };

PhaseVocoderEditor::PhaseVocoderEditor(mrta::BaseProcessor& p) :
    juce::AudioProcessorEditor(p),
    pluginProcessor { p },
    paramEditor(pluginProcessor.getParameterManager(), PARAM_HEIGHT)
{
    addAndMakeVisible(paramEditor);

    const auto HEIGHT { static_cast<int>(pluginProcessor.getParameterManager().getParameters().size() * PARAM_HEIGHT) };
    setSize(WIDTH, HEIGHT);
}

PhaseVocoderEditor::~PhaseVocoderEditor()
{
}

void PhaseVocoderEditor::paint(juce::Graphics&)
{
}

void PhaseVocoderEditor::resized()
{
    paramEditor.setBounds(getLocalBounds());
}

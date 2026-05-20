#include "PluginEditor.h"
#include "PluginProcessor.h"

AmpModelProcessorEditor::AmpModelProcessorEditor(mrta::BaseProcessor& p) :
    AudioProcessorEditor(&p),
    processor(p),
    genericParameterEditor(processor.getParameterManager())
{
    int height = static_cast<int>(processor.getParameterManager().getParameters().size())
               * genericParameterEditor.parameterWidgetHeight;
    setSize(300, height);
    addAndMakeVisible(genericParameterEditor);
}

AmpModelProcessorEditor::~AmpModelProcessorEditor()
{
}

void AmpModelProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AmpModelProcessorEditor::resized()
{
    genericParameterEditor.setBounds(getLocalBounds());
}

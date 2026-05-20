#include "PluginEditor.h"
#include <BinaryData.h>

// Width of the whole GUI
static constexpr int GUI_SIZE { 300 };

// Size of the knob
static constexpr int KNOB_SIZE { 200 };

MainProcessorEditor::MainProcessorEditor(MainProcessor& p) :
    juce::AudioProcessorEditor(p),
    pluginProcessor { p },
    knob(Param::ID::Drive, pluginProcessor.getParameterManager().getAPVTS(),
        juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize)),
    imag(juce::Image::PixelFormat::ARGB, 500, 500, true)
{
    addAndMakeVisible(knob);
    setSize(GUI_SIZE, GUI_SIZE);
}

MainProcessorEditor::~MainProcessorEditor()
{
}

void MainProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void MainProcessorEditor::resized()
{
    knob.setBounds(getLocalBounds().withSizeKeepingCentre(KNOB_SIZE, KNOB_SIZE));
}

#pragma once

#include "WaveNetProcessor.h"
#include "GenericParameterEditor.h"

class WaveNetProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    WaveNetProcessorEditor(WaveNetProcessor&);
    ~WaveNetProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& pluginProcessor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveNetProcessorEditor)
};

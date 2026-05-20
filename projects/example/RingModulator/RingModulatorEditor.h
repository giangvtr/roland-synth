#pragma once

#include <BaseProcessor.h>

#include <GenericParameterEditor.h>

class RingModulatorEditor final : public juce::AudioProcessorEditor
{
public:
    RingModulatorEditor(mrta::BaseProcessor&);
    ~RingModulatorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RingModulatorEditor)
};

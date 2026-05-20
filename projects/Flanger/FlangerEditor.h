#pragma once

#include <BaseProcessor.h>

#include <GenericParameterEditor.h>

class FlangerEditor final : public juce::AudioProcessorEditor
{
public:
    FlangerEditor(mrta::BaseProcessor&);
    ~FlangerEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FlangerEditor)
};

#pragma once

#include <BaseProcessor.h>

#include "GenericParameterEditor.h"

class CabinetSimulatorEditor final : public juce::AudioProcessorEditor
{
public:
    CabinetSimulatorEditor(mrta::BaseProcessor&);
    ~CabinetSimulatorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CabinetSimulatorEditor)
};

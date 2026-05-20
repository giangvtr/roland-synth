#pragma once

#include <BaseProcessor.h>

#include "GenericParameterEditor.h"

class OscillatorsEditor final : public juce::AudioProcessorEditor
{
public:
    OscillatorsEditor(mrta::BaseProcessor&);
    ~OscillatorsEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorsEditor)
};

#pragma once

#include <BaseProcessor.h>

#include "GenericParameterEditor.h"

class MidiHandlerEditor final : public juce::AudioProcessorEditor
{
public:
    MidiHandlerEditor(mrta::BaseProcessor&);
    ~MidiHandlerEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiHandlerEditor)
};

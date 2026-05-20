#pragma once

#include <BaseProcessor.h>

#include "GenericParameterEditor.h"

class AmpModelProcessor;

class AmpModelProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AmpModelProcessorEditor(mrta::BaseProcessor&);
    ~AmpModelProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;
    mrta::GenericParameterEditor genericParameterEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AmpModelProcessorEditor)
};

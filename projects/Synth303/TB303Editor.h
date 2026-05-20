#pragma once

#include <BaseProcessor.h>

#include "GenericParameterEditor.h"

class TB303Editor final : public juce::AudioProcessorEditor
{
public:
    TB303Editor(mrta::BaseProcessor&);
    ~TB303Editor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& processor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TB303Editor)
};

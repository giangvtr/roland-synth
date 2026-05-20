#pragma once

#include "BaseProcessor.h"
#include "GenericParameterEditor.h"

class PhaseVocoderEditor final : public juce::AudioProcessorEditor
{
public:
    PhaseVocoderEditor(mrta::BaseProcessor&);
    ~PhaseVocoderEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    mrta::BaseProcessor& pluginProcessor;

    mrta::GenericParameterEditor paramEditor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseVocoderEditor)
};

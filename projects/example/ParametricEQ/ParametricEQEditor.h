#pragma once

#include <GenericParameterEditor.h>
#include <BaseProcessor.h>
#include <MrtaLAF.h>

class ParametricEQEditor  : public juce::AudioProcessorEditor
{
public:
    ParametricEQEditor(mrta::BaseProcessor&);
    ~ParametricEQEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    static const int BandWidth { 250 };
    static const int ParamHeight { 80 };
    static const int ParamsPerBand { 4 };
    static const int NumOfBands { 3 };

private:
    mrta::BaseProcessor& processor;
    mrta::GenericParameterEditor band0ParameterEditor;
    mrta::GenericParameterEditor band1ParameterEditor;
    mrta::GenericParameterEditor band2ParameterEditor;

    GUI::MrtaLAF laf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParametricEQEditor)
};
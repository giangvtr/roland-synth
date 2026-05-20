#include "ParametricEQEditor.h"
#include "ParametricEQProcessor.h"

ParametricEQEditor::ParametricEQEditor(mrta::BaseProcessor& p) :
    juce::AudioProcessorEditor(p),
    processor { p },
    band0ParameterEditor(processor.getParameterManager(), ParamHeight,
                         { Param::ID::Band0Type, Param::ID::Band0Freq, Param::ID::Band0Reso, Param::ID::Band0Gain }),
    band1ParameterEditor(processor.getParameterManager(), ParamHeight,
                         { Param::ID::Band1Type, Param::ID::Band1Freq, Param::ID::Band1Reso, Param::ID::Band1Gain }),
    band2ParameterEditor(processor.getParameterManager(), ParamHeight,
                         { Param::ID::Band2Type, Param::ID::Band2Freq, Param::ID::Band2Reso, Param::ID::Band2Gain })
{
    addAndMakeVisible(band0ParameterEditor);
    addAndMakeVisible(band1ParameterEditor);
    addAndMakeVisible(band2ParameterEditor);

    band0ParameterEditor.setLookAndFeel(&laf);
    band2ParameterEditor.setLookAndFeel(&laf);

    setSize(NumOfBands * BandWidth, ParamsPerBand * ParamHeight);
}

ParametricEQEditor::~ParametricEQEditor()
{
    band0ParameterEditor.setLookAndFeel(nullptr);
    band2ParameterEditor.setLookAndFeel(nullptr);
}

void ParametricEQEditor::paint(juce::Graphics&)
{
}

void ParametricEQEditor::resized()
{
    auto localBounds { getLocalBounds() };
    band0ParameterEditor.setBounds(localBounds.removeFromLeft(BandWidth));
    band1ParameterEditor.setBounds(localBounds.removeFromLeft(BandWidth));
    band2ParameterEditor.setBounds(localBounds);
}


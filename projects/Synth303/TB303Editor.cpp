#include "TB303Editor.h"
#include "TB303Processor.h"

static constexpr int GUI_WIDTH { 920 };
static constexpr int GUI_HEIGHT { 360 };
static constexpr int TOP_ROW_HEIGHT { 220 };
static constexpr int BOTTOM_ROW_HEIGHT { 120 };
static constexpr int LABEL_HEIGHT { 20 };
static constexpr int KNOB_MARGIN { 12 };

static const juce::Colour PANEL_BACKGROUND { juce::Colour(0xFF1E1E1E) };
static const juce::Colour PANEL_INSET { juce::Colour(0xFF2E2E2E) };
static const juce::Colour PANEL_BORDER { juce::Colour(0xFF505050) };
static const juce::Colour LABEL_COLOUR { juce::Colours::white };
static const juce::Colour SWITCH_BACKGROUND { juce::Colour(0xFF242424) };
static const juce::Colour SWITCH_BORDER { juce::Colour(0xFF707070) };
static const juce::Colour SWITCH_TEXT { juce::Colours::white };

TB303LookAndFeel::TB303LookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, PANEL_BACKGROUND);
    setColour(juce::Slider::backgroundColourId, PANEL_INSET);
    setColour(juce::Slider::thumbColourId, juce::Colour(0xFFDE2B2B));
    setColour(juce::Slider::trackColourId, juce::Colour(0xFF828282));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF9B9B9B));
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF2D2D2D));
    setColour(juce::ComboBox::backgroundColourId, SWITCH_BACKGROUND);
    setColour(juce::ComboBox::outlineColourId, SWITCH_BORDER);
    setColour(juce::ComboBox::textColourId, SWITCH_TEXT);
    setColour(juce::Label::textColourId, LABEL_COLOUR);
}

void TB303LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider&)
{
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.45f;
    const auto centre = bounds.getCentre();
    const float outerStroke = radius * 0.12f;
    const float innerRadius = radius * 0.72f;

    g.setColour(juce::Colour(0xFF252525));
    g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour(juce::Colour(0xFF525252));
    g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, outerStroke * 0.85f);

    const int numTicks = 12;
    const float tickLength = radius * 0.18f;
    const float tickThickness = 1.4f;
    for (int i = 0; i < numTicks; ++i)
    {
        const float angle = rotaryStartAngle + (rotaryEndAngle - rotaryStartAngle) * (static_cast<float>(i) / static_cast<float>(numTicks - 1));
        const auto start = centre.getPointOnCircumference(radius - tickLength, angle);
        const auto end = centre.getPointOnCircumference(radius - 4.0f, angle);
        g.setColour(juce::Colour(0xFF8E8E8E));
        g.drawLine(start.x, start.y, end.x, end.y, tickThickness);
    }

    const float knobAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    const auto pointerStart = centre.getPointOnCircumference(innerRadius * 0.25f, knobAngle);
    const auto pointerEnd = centre.getPointOnCircumference(innerRadius, knobAngle);
    g.setColour(juce::Colour(0xFFDE2B2B));
    g.drawLine(pointerStart.x, pointerStart.y, pointerEnd.x, pointerEnd.y, outerStroke);

    g.setColour(juce::Colour(0xFF2F2F2F));
    g.fillEllipse(centre.x - innerRadius * 0.45f, centre.y - innerRadius * 0.45f, innerRadius * 0.9f, innerRadius * 0.9f);

    g.setColour(juce::Colour(0xFF7F7F7F));
    g.drawEllipse(centre.x - innerRadius * 0.45f, centre.y - innerRadius * 0.45f, innerRadius * 0.9f, innerRadius * 0.9f, 1.5f);
}

void TB303LookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float minSliderPos, float maxSliderPos,
                                          const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearHorizontal)
    {
        const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        const float trackHeight = bounds.getHeight() * 0.22f;
        const float trackY = bounds.getCentreY() - trackHeight * 0.5f;
        const auto trackRect = juce::Rectangle<float>(bounds.getX(), trackY, bounds.getWidth(), trackHeight);

        g.setColour(juce::Colour(0xFF2D2D2D));
        g.fillRoundedRectangle(trackRect, trackHeight * 0.5f);

        const auto fillRect = juce::Rectangle<float>(trackRect.getX(), trackRect.getY(), sliderPos - trackRect.getX(), trackRect.getHeight());
        g.setColour(juce::Colour(0xFFDE2B2B));
        g.fillRoundedRectangle(fillRect, trackHeight * 0.5f);

        const float thumbRadius = trackHeight * 1.7f;
        const float thumbX = (sliderPos - minSliderPos) / (maxSliderPos - minSliderPos) * trackRect.getWidth() + trackRect.getX();
        const auto thumbCentre = juce::Point<float>(thumbX, trackRect.getCentreY());

        g.setColour(juce::Colour(0xFF1E1E1E));
        g.fillEllipse(thumbCentre.x - thumbRadius, thumbCentre.y - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f);
        g.setColour(juce::Colour(0xFF8E8E8E));
        g.drawEllipse(thumbCentre.x - thumbRadius, thumbCentre.y - thumbRadius, thumbRadius * 2.0f, thumbRadius * 2.0f, 1.7f);
    }
    else
    {
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

TB303Editor::TB303Editor(mrta::BaseProcessor& p) :
    juce::AudioProcessorEditor(p),
    processor(p),
    tuningSlider(Param::ID::Tuning, processor.getParameterManager().getAPVTS()),
    cutoffSlider(Param::ID::Cutoff, processor.getParameterManager().getAPVTS()),
    resonanceSlider(Param::ID::Resonance, processor.getParameterManager().getAPVTS()),
    envelopeModSlider(Param::ID::EnvelopeMod, processor.getParameterManager().getAPVTS()),
    decaySlider(Param::ID::Decay, processor.getParameterManager().getAPVTS()),
    accentSlider(Param::ID::Accent, processor.getParameterManager().getAPVTS()),
    volumeSlider(Param::ID::Volume, processor.getParameterManager().getAPVTS()),
    waveformButton("Wave")
{
    setLookAndFeel(&customLookAndFeel);

    tuningSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    envelopeModSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    accentSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 18);

    waveformButton.setColour(juce::TextButton::buttonColourId, SWITCH_BACKGROUND);
    waveformButton.setColour(juce::TextButton::textColourOffId, SWITCH_TEXT);
    waveformButton.setColour(juce::TextButton::textColourOnId, SWITCH_TEXT);
    waveformButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFDE2B2B));

    tuningLabel.setText("Tuning", juce::dontSendNotification);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    envModLabel.setText("Env Mod", juce::dontSendNotification);
    decayLabel.setText("Decay", juce::dontSendNotification);
    accentLabel.setText("Accent", juce::dontSendNotification);
    waveformLabel.setText("Wave", juce::dontSendNotification);
    volumeLabel.setText("Volume", juce::dontSendNotification);

    addAndMakeVisible(tuningSlider);
    addAndMakeVisible(cutoffSlider);
    addAndMakeVisible(resonanceSlider);
    addAndMakeVisible(envelopeModSlider);
    addAndMakeVisible(decaySlider);
    addAndMakeVisible(accentSlider);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(waveformButton);

    setupLabel(tuningLabel);
    setupLabel(cutoffLabel);
    setupLabel(resonanceLabel);
    setupLabel(envModLabel);
    setupLabel(decayLabel);
    setupLabel(accentLabel);
    setupLabel(waveformLabel);
    setupLabel(volumeLabel);

    updateWaveformButtonText();

    auto& apvts = processor.getParameterManager().getAPVTS();
    waveformButton.onClick = [this, &apvts]()
    {
        if (auto* parameter = apvts.getParameter(Param::ID::Waveform))
        {
            const float currentValue = parameter->getValue();
            const float nextValue = (std::round(currentValue) == 0.0f) ? 1.0f : 0.0f;
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(nextValue);
            parameter->endChangeGesture();
        }
        updateWaveformButtonText();
    };

    updateWaveformButtonText();

    setSize(GUI_WIDTH, GUI_HEIGHT);
}

void TB303Editor::updateWaveformButtonText()
{
    auto& apvts = processor.getParameterManager().getAPVTS();
    if (auto* parameter = apvts.getParameter(Param::ID::Waveform))
    {
        const int index = static_cast<int>(std::round(parameter->getValue()));
        waveformButton.setButtonText(index == 0 ? "Saw" : "Square");
    }
}

TB303Editor::~TB303Editor()
{
    setLookAndFeel(nullptr);
}

void TB303Editor::setupLabel(juce::Label& label)
{
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, LABEL_COLOUR);
    addAndMakeVisible(label);
}

void TB303Editor::paint(juce::Graphics& g)
{
    g.fillAll(PANEL_BACKGROUND);
    g.setColour(PANEL_INSET);
    g.fillRoundedRectangle(getLocalBounds().reduced(10).toFloat(), 18.0f);
    g.setColour(PANEL_BORDER);
    g.drawRoundedRectangle(getLocalBounds().reduced(10).toFloat(), 18.0f, 3.0f);
}

void TB303Editor::resized()
{
    auto bounds = getLocalBounds().reduced(16);
    auto topRow = bounds.removeFromTop(TOP_ROW_HEIGHT);

    const int knobCount = 6;
    const int knobWidth = topRow.getWidth() / knobCount;
    for (int i = 0; i < knobCount; ++i)
    {
        auto cell = topRow.removeFromLeft(knobWidth).reduced(KNOB_MARGIN, 8);
        auto sliderArea = cell.removeFromTop(cell.getHeight() - LABEL_HEIGHT - 6);
        auto labelArea = cell.removeFromTop(LABEL_HEIGHT);

        switch (i)
        {
            case 0:
                tuningSlider.setBounds(sliderArea);
                tuningLabel.setBounds(labelArea);
                break;
            case 1:
                cutoffSlider.setBounds(sliderArea);
                cutoffLabel.setBounds(labelArea);
                break;
            case 2:
                resonanceSlider.setBounds(sliderArea);
                resonanceLabel.setBounds(labelArea);
                break;
            case 3:
                envelopeModSlider.setBounds(sliderArea);
                envModLabel.setBounds(labelArea);
                break;
            case 4:
                decaySlider.setBounds(sliderArea);
                decayLabel.setBounds(labelArea);
                break;
            case 5:
                accentSlider.setBounds(sliderArea);
                accentLabel.setBounds(labelArea);
                break;
        }
    }

    auto bottomRow = bounds.removeFromTop(BOTTOM_ROW_HEIGHT).reduced(10, 10);
    auto switchArea = bottomRow.removeFromLeft(bottomRow.getWidth() / 3).reduced(12, 12);
    auto volumeArea = bottomRow.reduced(12, 18);

    waveformButton.setBounds(switchArea.removeFromTop(switchArea.getHeight() - LABEL_HEIGHT));
    waveformLabel.setBounds(switchArea.removeFromTop(LABEL_HEIGHT));

    auto volumeLabelArea = volumeArea.removeFromTop(LABEL_HEIGHT);
    auto volumeSliderArea = volumeArea.reduced(0, 6);
    volumeSlider.setBounds(volumeSliderArea);
    volumeLabel.setBounds(volumeLabelArea);
}

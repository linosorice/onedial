#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ZoneBlender.h"

namespace
{
    const juce::Colour kBackground{0xFF0D0D0D};
    const juce::Colour kPanelDark{0xFF1A1A1A};
    const juce::Colour kPanelLight{0xFF222222};
    const juce::Colour kCream{0xFFE8DCC8};
    const juce::Colour kGold{0xFFD4A857};
    const juce::Colour kGoldDim{0xFF8A6E3A};
    const juce::Colour kKnobBody{0xFF2C2C2C};
    const juce::Colour kKnobRing{0xFF3A3A3A};
    const juce::Colour kRedVU{0xFFCC3333};
    const juce::Colour kGreenVU{0xFF66AA44};
}

// --- AnalogLookAndFeel ---

AnalogLookAndFeel::AnalogLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, kCream);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ToggleButton::textColourId, kCream.withAlpha(0.5f));
    setColour(juce::ToggleButton::tickColourId, kGold);
}

void AnalogLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    const float radius = static_cast<float>(juce::jmin(width, height)) / 2.0f - 4.0f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const bool isMainDial = (slider.getName() == "Dial" || radius > 60.0f);

    // Outer shadow
    if (isMainDial)
    {
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse(centreX - radius - 4, centreY - radius - 2, radius * 2 + 8, radius * 2 + 8);
    }

    // Knob body - dark metallic
    {
        juce::ColourGradient bodyGrad(kKnobBody.brighter(0.15f), centreX, centreY - radius,
                                       kKnobBody.darker(0.2f), centreX, centreY + radius, false);
        g.setGradientFill(bodyGrad);
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
    }

    // Knob ring / bevel
    g.setColour(kKnobRing);
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    // Inner circle texture (concentric rings for grip)
    if (isMainDial)
    {
        g.setColour(juce::Colours::white.withAlpha(0.03f));
        for (float r = radius * 0.3f; r < radius * 0.85f; r += 4.0f)
            g.drawEllipse(centreX - r, centreY - r, r * 2.0f, r * 2.0f, 0.5f);
    }

    // Arc track (background)
    {
        juce::Path arcBg;
        arcBg.addCentredArc(centreX, centreY, radius + 6, radius + 6,
                            0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(kPanelLight);
        g.strokePath(arcBg, juce::PathStrokeType(3.0f));
    }

    // Arc fill (gold indicator)
    {
        juce::Path arcFill;
        arcFill.addCentredArc(centreX, centreY, radius + 6, radius + 6,
                              0.0f, rotaryStartAngle, angle, true);
        g.setColour(isMainDial ? kGold : kGoldDim);
        g.strokePath(arcFill, juce::PathStrokeType(3.0f));
    }

    // Pointer / indicator line
    {
        const float pointerLength = radius * 0.75f;
        const float pointerThickness = isMainDial ? 3.0f : 2.0f;
        juce::Path pointer;
        pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(kGold);
        g.fillPath(pointer);

        // Pointer dot at tip
        float dotX = centreX + std::sin(angle) * (radius * 0.7f);
        float dotY = centreY - std::cos(angle) * (radius * 0.7f);
        g.setColour(kGold.brighter(0.3f));
        g.fillEllipse(dotX - 3.0f, dotY - 3.0f, 6.0f, 6.0f);
    }

    // Center cap
    {
        float capRadius = isMainDial ? radius * 0.18f : radius * 0.22f;
        juce::ColourGradient capGrad(kKnobBody.brighter(0.3f), centreX, centreY - capRadius,
                                      kKnobBody, centreX, centreY + capRadius, false);
        g.setGradientFill(capGrad);
        g.fillEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f);
        g.setColour(kKnobRing.brighter(0.1f));
        g.drawEllipse(centreX - capRadius, centreY - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.0f);
    }
}

// --- VU Meter ---

VUMeter::VUMeter(OneDial& p) : processor(p)
{
    startTimerHz(30);
}

void VUMeter::timerCallback()
{
    float currentLevel = (processor.getOutputLevelLeft() + processor.getOutputLevelRight()) * 0.5f;
    // Smooth with decay
    if (currentLevel > smoothedLevel)
        smoothedLevel = currentLevel;
    else
        smoothedLevel *= 0.92f;

    repaint();
}

void VUMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // VU meter background panel
    g.setColour(kPanelDark);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(kKnobRing);
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    // "VU" label
    g.setColour(kCream.withAlpha(0.3f));
    g.setFont(juce::FontOptions(9.0f));
    g.drawText("VU", bounds.reduced(4, 2), juce::Justification::topLeft);

    // Meter arc (analog needle style)
    float meterArea = bounds.getWidth() - 16.0f;
    float centreX = bounds.getCentreX();
    float arcY = bounds.getBottom() - 8.0f;
    float arcRadius = meterArea * 0.45f;

    // Scale markings
    g.setColour(kCream.withAlpha(0.2f));
    float startAngle = juce::MathConstants<float>::pi * 1.2f;
    float endAngle = juce::MathConstants<float>::pi * 1.8f;

    for (int i = 0; i <= 10; ++i)
    {
        float t = static_cast<float>(i) / 10.0f;
        float a = startAngle + t * (endAngle - startAngle);
        float innerR = arcRadius - 6.0f;
        float outerR = arcRadius;
        float x1 = centreX + std::cos(a) * innerR;
        float y1 = arcY + std::sin(a) * innerR;
        float x2 = centreX + std::cos(a) * outerR;
        float y2 = arcY + std::sin(a) * outerR;

        g.setColour(i >= 8 ? kRedVU.withAlpha(0.4f) : kCream.withAlpha(0.15f));
        g.drawLine(x1, y1, x2, y2, 1.0f);
    }

    // Needle
    float levelDb = juce::Decibels::gainToDecibels(smoothedLevel, -60.0f);
    float normalized = juce::jmap(levelDb, -60.0f, 0.0f, 0.0f, 1.0f);
    normalized = juce::jlimit(0.0f, 1.0f, normalized);

    float needleAngle = startAngle + normalized * (endAngle - startAngle);
    float needleLen = arcRadius - 2.0f;
    float nx = centreX + std::cos(needleAngle) * needleLen;
    float ny = arcY + std::sin(needleAngle) * needleLen;

    g.setColour(normalized > 0.8f ? kRedVU : kGold);
    g.drawLine(centreX, arcY, nx, ny, 1.5f);

    // Pivot dot
    g.setColour(kGold);
    g.fillEllipse(centreX - 2.0f, arcY - 2.0f, 4.0f, 4.0f);
}

// --- Editor ---

OneDialEditor::OneDialEditor(OneDial& p)
    : AudioProcessorEditor(&p), processor(p), vuMeter(p)
{
    setSize(380, 520);
    setLookAndFeel(&analogLnf);

    // Main dial
    dialKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    dialKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
    dialKnob.onValueChange = [this] { updateZoneLabel(); };
    addAndMakeVisible(dialKnob);

    // Zone label
    zoneLabel.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    zoneLabel.setColour(juce::Label::textColourId, kGold);
    zoneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(zoneLabel);

    // Input knob
    inputKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    inputKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(inputKnob);

    inputLabel.setText("IN", juce::dontSendNotification);
    inputLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    inputLabel.setColour(juce::Label::textColourId, kCream.withAlpha(0.4f));
    inputLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(inputLabel);

    // Output knob
    outputKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    outputKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(outputKnob);

    outputLabel.setText("OUT", juce::dontSendNotification);
    outputLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    outputLabel.setColour(juce::Label::textColourId, kCream.withAlpha(0.4f));
    outputLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outputLabel);

    // Bypass
    addAndMakeVisible(bypassButton);

    // VU meter
    addAndMakeVisible(vuMeter);

    // APVTS attachments
    dialAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "dial", dialKnob);
    inputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "inputGain", inputKnob);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "outputGain", outputKnob);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.getAPVTS(), "bypass", bypassButton);

    updateZoneLabel();
}

OneDialEditor::~OneDialEditor()
{
    setLookAndFeel(nullptr);
}

void OneDialEditor::paint(juce::Graphics& g)
{
    // Background - dark with subtle gradient
    {
        juce::ColourGradient bg(kBackground.brighter(0.05f), 0, 0,
                                 kBackground, 0, static_cast<float>(getHeight()), false);
        g.setGradientFill(bg);
        g.fillAll();
    }

    auto w = static_cast<float>(getWidth());

    // Top panel strip
    g.setColour(kPanelDark);
    g.fillRect(0.0f, 0.0f, w, 38.0f);
    g.setColour(kGold.withAlpha(0.15f));
    g.drawLine(0.0f, 38.0f, w, 38.0f, 1.0f);

    // Title
    g.setColour(kCream);
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText("ONEDIAL", 0, 6, static_cast<int>(w), 28, juce::Justification::centred);

    // Zone tick marks around the main dial
    auto dialArea = getLocalBounds().reduced(50, 0);
    dialArea.removeFromTop(48);
    dialArea = dialArea.removeFromTop(260);
    auto centre = dialArea.getCentre().toFloat();
    float radius = static_cast<float>(dialArea.getWidth()) / 2.0f + 8.0f;

    float startAngle = juce::MathConstants<float>::pi * 1.25f;
    float range = juce::MathConstants<float>::pi * 1.5f;

    // Zone boundary ticks and numbers
    for (int i = 0; i <= 10; ++i)
    {
        float normalized = static_cast<float>(i) / 10.0f;
        float angle = startAngle + normalized * range;

        bool isMajor = (i % 2 == 0);
        float tickLen = isMajor ? 10.0f : 5.0f;
        float tickInner = radius;
        float tickOuter = radius + tickLen;

        float x1 = centre.x + std::cos(angle) * tickInner;
        float y1 = centre.y + std::sin(angle) * tickInner;
        float x2 = centre.x + std::cos(angle) * tickOuter;
        float y2 = centre.y + std::sin(angle) * tickOuter;

        g.setColour(isMajor ? kCream.withAlpha(0.5f) : kCream.withAlpha(0.2f));
        g.drawLine(x1, y1, x2, y2, isMajor ? 1.5f : 1.0f);

        if (isMajor)
        {
            float labelRadius = radius + tickLen + 10.0f;
            float lx = centre.x + std::cos(angle) * labelRadius;
            float ly = centre.y + std::sin(angle) * labelRadius;
            g.setColour(kCream.withAlpha(0.4f));
            g.setFont(juce::FontOptions(10.0f));
            g.drawText(juce::String(i), static_cast<int>(lx) - 10, static_cast<int>(ly) - 7, 20, 14,
                       juce::Justification::centred);
        }
    }

    // Bottom separator
    float bottomY = static_cast<float>(getHeight()) - 110.0f;
    g.setColour(kGold.withAlpha(0.1f));
    g.drawLine(20.0f, bottomY, w - 20.0f, bottomY, 1.0f);
}

void OneDialEditor::resized()
{
    auto bounds = getLocalBounds();

    // Title area
    bounds.removeFromTop(42);

    // Main dial
    auto dialArea = bounds.removeFromTop(260);
    dialKnob.setBounds(dialArea.reduced(55, 10));

    // Zone label
    auto labelArea = bounds.removeFromTop(36);
    zoneLabel.setBounds(labelArea);

    // VU meter
    auto vuArea = bounds.removeFromTop(50);
    vuMeter.setBounds(vuArea.reduced(60, 2));

    // Bottom controls
    bounds.removeFromTop(8);
    auto bottomArea = bounds.removeFromTop(75);

    int thirdW = bottomArea.getWidth() / 3;
    auto leftCol = bottomArea.removeFromLeft(thirdW);
    auto rightCol = bottomArea.removeFromRight(thirdW);
    auto centerCol = bottomArea;

    // Input knob
    inputKnob.setBounds(leftCol.reduced(20, 5).removeFromTop(50));
    inputLabel.setBounds(leftCol.removeFromBottom(18));

    // Output knob
    outputKnob.setBounds(rightCol.reduced(20, 5).removeFromTop(50));
    outputLabel.setBounds(rightCol.removeFromBottom(18));

    // Bypass
    bypassButton.setBounds(centerCol.reduced(15, 22));
}

void OneDialEditor::updateZoneLabel()
{
    float dialValue = static_cast<float>(dialKnob.getValue());
    zoneLabel.setText(ZoneBlender::getZoneName(dialValue), juce::dontSendNotification);
}

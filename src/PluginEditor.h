#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class Timbro;

// =============================================================================
// Flat, Surge XT-inspired LookAndFeel — drawn entirely with juce::Graphics.
// VOICE is rendered slightly differently (zone tick marks on the ring) when
// the slider's componentID is "voice".
// =============================================================================
class TimbroLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TimbroLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
};


// Slider subclass that triggers a repaint on mouse enter/exit so the
// LookAndFeel can render a hover state via juce::Slider::isMouseOver().
class HoverableSlider : public juce::Slider
{
public:
    using juce::Slider::Slider;
    void mouseEnter(const juce::MouseEvent& e) override
    {
        juce::Slider::mouseEnter(e);
        repaint();
    }
    void mouseExit(const juce::MouseEvent& e) override
    {
        juce::Slider::mouseExit(e);
        repaint();
    }
};


// 5 flat dots above the VOICE knob plus a dynamic zone-name label.
// Dot brightness interpolates between two adjacent zones during a blend;
// the label tracks the dominant zone.
class ZoneIndicator : public juce::Component, private juce::Timer
{
public:
    explicit ZoneIndicator(juce::AudioProcessorValueTreeState& apvts);
    ~ZoneIndicator() override;

    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;

    juce::AudioProcessorValueTreeState& apvts;
    float lastDialValue = -1.0f;

    JUCE_DECLARE_NON_COPYABLE(ZoneIndicator)
};


class TimbroEditor : public juce::AudioProcessorEditor
{
public:
    explicit TimbroEditor(Timbro&);
    ~TimbroEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    Timbro& processor;
    TimbroLookAndFeel lnf;

    // Four controls: BYPASS toggle, INPUT/OUTPUT knobs, VOICE knob (bigger).
    juce::ToggleButton bypassButton{"BYPASS"};
    HoverableSlider inputKnob;
    HoverableSlider outputKnob;
    HoverableSlider voiceKnob;

    // Numeric readouts under INPUT and OUTPUT. VOICE uses the ZoneIndicator
    // as its readout instead, so no separate label is needed there.
    juce::Label inputValueLabel;
    juce::Label outputValueLabel;

    ZoneIndicator zoneIndicator;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  inputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>  voiceAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimbroEditor)
};

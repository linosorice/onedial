#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class OneDial;

// Custom look and feel for analog 70s aesthetic
class AnalogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnalogLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override;
};

// Decorative VU meter component
class VUMeter : public juce::Component, private juce::Timer
{
public:
    VUMeter(OneDial& processor);
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;
    OneDial& processor;
    float smoothedLevel = 0.0f;
};

class OneDialEditor : public juce::AudioProcessorEditor
{
public:
    explicit OneDialEditor(OneDial&);
    ~OneDialEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    OneDial& processor;
    AnalogLookAndFeel analogLnf;

    // Main dial
    juce::Slider dialKnob;
    juce::Label zoneLabel;

    // Secondary controls
    juce::Slider inputKnob;
    juce::Slider outputKnob;
    juce::Label inputLabel;
    juce::Label outputLabel;

    // Bypass
    juce::ToggleButton bypassButton{"BYPASS"};

    // VU Meter
    VUMeter vuMeter;

    // APVTS attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dialAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    void updateZoneLabel();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OneDialEditor)
};

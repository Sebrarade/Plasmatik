#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PlasmaPercAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PlasmaPercAudioProcessorEditor (PlasmaPercAudioProcessor&);
    ~PlasmaPercAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    struct EngineStrip
    {
        juce::Label title;
        std::array<juce::Slider, 4> knobs;
        std::array<juce::Label, 4> labels;
        std::array<std::unique_ptr<SliderAttachment>, 4> attachments;
    };

    struct SeqStrip
    {
        juce::Label title;
        juce::ComboBox division;
        juce::Label divisionL;
        juce::Slider density;
        juce::Label densityL;
        juce::Slider rotate;
        juce::Label rotateL;
        std::unique_ptr<ComboAttachment> divA;
        std::unique_ptr<SliderAttachment> densityA;
        std::unique_ptr<SliderAttachment> rotateA;
    };

    struct EverythingStrip
    {
        juce::Label title;
        juce::Slider knob;
        juce::Label label;
        std::unique_ptr<SliderAttachment> attachment;
    };

    void styleKnob (juce::Slider& s, juce::Label& l, const juce::String& name, bool large = false);
    void styleCombo (juce::ComboBox& box, juce::Label& l, const juce::String& name);
    void drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title);

    PlasmaPercAudioProcessor& processor;
    std::array<SeqStrip, 4> seqs;
    std::array<EngineStrip, 4> engines;
    std::array<EverythingStrip, 4> macros;

    std::array<juce::Slider, 10> globals;
    std::array<juce::Label, 10> globalLabels;
    std::array<std::unique_ptr<SliderAttachment>, 10> globalAttachments;

    juce::TextButton mutate { "MUTATE" };
    juce::Label subtitle;
    juce::Image fairyImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlasmaPercAudioProcessorEditor)
};

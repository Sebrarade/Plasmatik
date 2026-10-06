#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PlasmaPercAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PlasmaPercAudioProcessorEditor (PlasmaPercAudioProcessor&);
    ~PlasmaPercAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    class PlasmatikLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider (juce::Graphics&, int, int, int, int, float,
                               float, float, juce::Slider&) override;
        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                                   bool, bool) override;
        void drawLinearSlider (juce::Graphics&, int, int, int, int, float, float, float,
                               const juce::Slider::SliderStyle, juce::Slider&) override;
        void drawComboBox (juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
        void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    };

    class PatternView : public juce::Component, private juce::Timer
    {
    public:
        PatternView (PlasmaPercAudioProcessor& p, int voiceIndex, juce::Colour colour);
        void paint (juce::Graphics&) override;

    private:
        void timerCallback() override { repaint(); }
        PlasmaPercAudioProcessor& processor;
        int voice = 0;
        juce::Colour accent;
    };

    struct SeqStrip
    {
        juce::Label title;
        juce::ComboBox division;
        juce::TextButton evolve { "EVOLVE" };
        juce::TextButton drift { "DRIFT" };
        juce::TextButton autoMode { "AUTO" };

        juce::Slider density;
        juce::Slider prob;
        juce::Slider rotate;
        juce::Label densityL;
        juce::Label probL;
        juce::Label rotateL;

        std::unique_ptr<PatternView> pattern;
        std::unique_ptr<ComboAttachment> divA;
        std::unique_ptr<SliderAttachment> densityA;
        std::unique_ptr<SliderAttachment> probA;
        std::unique_ptr<SliderAttachment> rotateA;
        std::unique_ptr<ButtonAttachment> driftA;
        std::unique_ptr<ButtonAttachment> autoA;
    };

    struct EngineStrip
    {
        juce::Label title;
        juce::TextButton mute { "MUTE" };

        std::array<juce::Slider, 4> knobs;
        std::array<juce::Label, 4> labels;
        std::array<std::unique_ptr<SliderAttachment>, 4> attachments;

        juce::ComboBox wave;
        juce::Label waveL;
        juce::Slider level;
        juce::Label levelL;

        std::unique_ptr<ButtonAttachment> muteA;
        std::unique_ptr<ComboAttachment> waveA;
        std::unique_ptr<SliderAttachment> levelA;
    };

    struct EverythingStrip
    {
        juce::Label title;
        juce::Slider amount;
        juce::Label amountL;
        juce::ComboBox shape;
        juce::Label shapeL;
        juce::ComboBox rate;
        juce::Label rateL;

        std::unique_ptr<SliderAttachment> amountA;
        std::unique_ptr<ComboAttachment> shapeA;
        std::unique_ptr<ComboAttachment> rateA;
    };

    void styleKnob (juce::Slider&, juce::Label&, const juce::String&, juce::Colour, bool large = false);
    void styleSmallKnob (juce::Slider&, juce::Label&, const juce::String&, juce::Colour);
    void styleHorizontal (juce::Slider&, juce::Label&, const juce::String&, juce::Colour);
    void styleCombo (juce::ComboBox&, juce::Label&, const juce::String&);
    void styleActionButton (juce::TextButton&, juce::Colour accent, bool toggle);
    void drawPanel (juce::Graphics&, juce::Rectangle<int>, const juce::String&) const;
    juce::Colour voiceColour (int index) const;

    PlasmaPercAudioProcessor& processor;
    PlasmatikLookAndFeel lookAndFeel;

    std::array<SeqStrip, 4> seqs;
    std::array<EngineStrip, 4> engines;
    std::array<EverythingStrip, 4> macros;

    std::array<juce::Slider, 10> globals;
    std::array<juce::Label, 10> globalLabels;
    std::array<std::unique_ptr<SliderAttachment>, 10> globalAttachments;

    std::array<juce::Slider, 4> crusherKnobs;
    std::array<juce::Label, 4> crusherLabels;
    std::array<std::unique_ptr<SliderAttachment>, 4> crusherAttachments;
    juce::ComboBox crusherShape;
    juce::Label crusherShapeL;
    juce::ComboBox crusherRate;
    juce::Label crusherRateL;
    std::unique_ptr<ComboAttachment> crusherShapeA;
    std::unique_ptr<ComboAttachment> crusherRateA;

    juce::Slider output;
    juce::Label outputL;
    juce::Slider width;
    juce::Label widthL;
    std::unique_ptr<SliderAttachment> outputA;
    std::unique_ptr<SliderAttachment> widthA;

    juce::TextButton mutate { "MUTATE" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlasmaPercAudioProcessorEditor)
};

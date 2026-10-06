#pragma once
#include <JuceHeader.h>
#include "PlasmaVoice.h"
#include "Euclid.h"

class PlasmaPercAudioProcessor : public juce::AudioProcessor
{
public:
    PlasmaPercAudioProcessor();
    ~PlasmaPercAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    void mutate (float amount = 0.72f);

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    PlasmaVoiceParams readVoiceParams (int index, double bpm) const;
    bool shouldTriggerVoice (int voice, int step, float interactBoost);
    static int divisionInterval (int choice) noexcept;

    juce::AudioProcessorValueTreeState apvts;
    std::array<PlasmaVoice, 4> voices;
    juce::Random rng;

    double currentSampleRate = 44100.0;
    double masterPhaseSamples = 0.0;
    int master32Step = 0;
    std::array<int, 4> localSteps { 0, 0, 0, 0 };
    std::array<int, 4> cycleCounts { 0, 0, 0, 0 };
    std::array<bool, 4> previousHits { false, false, false, false };
    uint32_t triggerCounter = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlasmaPercAudioProcessor)
};

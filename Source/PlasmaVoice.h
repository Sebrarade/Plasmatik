#pragma once
#include <JuceHeader.h>

struct PlasmaVoiceParams
{
    float pitchHz = 180.0f;
    float tone = 0.5f;
    float fm = 0.35f;
    float noise = 0.25f;
    float level = 0.85f;
    int waveShape = 0;

    float decay = 0.35f;
    float envAmount = 0.75f;
    float filterBase = 1800.0f;
    int filterType = 0; // 0 LP, 1 BP, 2 HP
    float filterBlast = 0.75f;
    float resonance = 0.65f;
    float drive = 0.45f;
    float chaos = 0.25f;

    float everythingAmount = 0.4f;
    int everythingShape = 0;
    float everythingRateHz = 2.0f;
};

class PlasmaVoice
{
public:
    void prepare (double sampleRate, int maximumBlockSize);
    void reset();
    void trigger (float velocity, const PlasmaVoiceParams& p, float globalChaos, uint32_t seed);
    void process (juce::AudioBuffer<float>& output, int startSample, int numSamples,
                  const PlasmaVoiceParams& p, float globalChaos, int voiceIndex);

private:
    static float fastTanh (float x) noexcept { return std::tanh (x); }
    static float softFold (float x) noexcept;
    float renderCarrier (float phase, int shape, float noiseSample) noexcept;
    float nextEverythingLfo (int shape, float rateHz);

    double sr = 44100.0;
    bool active = false;
    float ampEnv = 0.0f;
    float filterEnv = 0.0f;
    float phase = 0.0f;
    float modPhase = 0.0f;
    float vel = 1.0f;
    float randomOffset = 0.0f;
    float pitchJitter = 0.0f;

    float lfoPhase = 0.0f;
    float sampleHoldValue = 0.0f;
    float smoothRandomFrom = 0.0f;
    float smoothRandomTo = 0.0f;

    float f1 = 0.0f;
    float f2 = 0.0f;
    float f3 = 0.0f;
    float f4 = 0.0f;

    juce::Random rng;
};

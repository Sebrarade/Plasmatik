#include "PlasmaVoice.h"

float PlasmaVoice::softFold (float x) noexcept
{
    x = std::fmod (x + 3.0f, 4.0f) - 2.0f;
    if (x > 1.0f) x = 2.0f - x;
    if (x < -1.0f) x = -2.0f - x;
    return fastTanh (x * 1.35f);
}

float PlasmaVoice::renderCarrier (float ph, int shape, float noiseSample) noexcept
{
    const float norm = ph / juce::MathConstants<float>::twoPi;
    const float frac = norm - std::floor (norm);

    switch (shape)
    {
        case 1: return 1.0f - 4.0f * std::abs (frac - 0.5f); // triangle
        case 2: return frac < 0.5f ? 1.0f : -1.0f;           // square
        case 3: return 2.0f * frac - 1.0f;                  // saw
        case 4: return noiseSample;                         // noise oscillator
        default: return std::sin (ph);                      // sine
    }
}

float PlasmaVoice::nextEverythingLfo (int shape, float rateHz)
{
    const float inc = juce::jmax (0.0000001f, rateHz / (float) sr);
    const float oldPhase = lfoPhase;
    lfoPhase += inc;
    bool wrapped = false;

    if (lfoPhase >= 1.0f)
    {
        lfoPhase -= std::floor (lfoPhase);
        wrapped = true;
    }

    if (wrapped)
    {
        sampleHoldValue = rng.nextFloat() * 2.0f - 1.0f;
        smoothRandomFrom = smoothRandomTo;
        smoothRandomTo = rng.nextFloat() * 2.0f - 1.0f;
    }

    const float p = lfoPhase;

    switch (shape)
    {
        case 1: return 1.0f - 4.0f * std::abs (p - 0.5f);                    // triangle
        case 2: return 2.0f * p - 1.0f;                                      // saw
        case 3: return p < 0.5f ? 1.0f : -1.0f;                              // square
        case 4: return sampleHoldValue;                                       // sample & hold
        case 5:
        {
            const float t = p * p * (3.0f - 2.0f * p);                        // smoothstep
            return smoothRandomFrom + (smoothRandomTo - smoothRandomFrom) * t; // smooth random
        }
        default: return std::sin (juce::MathConstants<float>::twoPi * p);     // sine
    }
}

void PlasmaVoice::prepare (double sampleRate, int)
{
    sr = sampleRate;
    reset();
}

void PlasmaVoice::reset()
{
    active = false;
    ampEnv = filterEnv = 0.0f;
    phase = modPhase = 0.0f;
    lfoPhase = 0.0f;
    sampleHoldValue = 0.0f;
    smoothRandomFrom = 0.0f;
    smoothRandomTo = 0.0f;
    f1 = f2 = f3 = f4 = 0.0f;
}

void PlasmaVoice::trigger (float velocity, const PlasmaVoiceParams&, float globalChaos, uint32_t seed)
{
    rng.setSeed ((juce::int64) seed + 0x9e3779b9);
    vel = juce::jlimit (0.0f, 1.0f, velocity);
    ampEnv = 1.0f;
    filterEnv = 1.0f;
    active = true;
    phase = 0.0f;
    modPhase = rng.nextFloat() * juce::MathConstants<float>::twoPi;
    randomOffset = rng.nextFloat() * 2.0f - 1.0f;
    pitchJitter = (rng.nextFloat() * 2.0f - 1.0f) * (0.018f + globalChaos * 0.12f);
}

void PlasmaVoice::process (juce::AudioBuffer<float>& output, int startSample, int numSamples,
                           const PlasmaVoiceParams& p, float globalChaos, int voiceIndex)
{
    auto* left = output.getWritePointer (0);
    auto* right = output.getNumChannels() > 1 ? output.getWritePointer (1) : left;

    const float chaos = juce::jlimit (0.0f, 1.0f, globalChaos + p.chaos * 0.45f);
    const float macro = juce::jlimit (0.0f, 1.0f, p.everythingAmount);
    const float baseDecaySeconds = juce::jmap (p.decay, 0.03f, 1.8f);

    for (int n = 0; n < numSamples; ++n)
    {
        const float everything = nextEverythingLfo (p.everythingShape, p.everythingRateHz);
        const float macroA = everything * macro;
        const float macroB = fastTanh ((everything + randomOffset * 0.22f) * (0.9f + macro * 0.8f));

        if (! active)
            continue;

        const float dynamicTone = juce::jlimit (0.0f, 1.0f, p.tone + macroA * 0.48f);
        const float dynamicFm = juce::jlimit (0.0f, 1.0f, p.fm + macroB * 0.68f);
        const float dynamicNoise = juce::jlimit (0.0f, 1.0f, p.noise - macroA * 0.48f);
        const float dynamicLevel = juce::jlimit (0.08f, 1.0f, p.level + macroB * 0.26f);

        // Everything does NOT modulate decay by design.
        const float envPitchOct = (2.1f + 2.2f * p.envAmount + p.filterBlast * 0.8f) * filterEnv
                                  + macroA * 0.90f;
        const float base = p.pitchHz * (1.0f + pitchJitter + macroB * 0.12f);
        const float freq = juce::jlimit (20.0f, 12000.0f, base * std::pow (2.0f, envPitchOct));

        const float modFreq = freq * (1.15f + 2.8f * dynamicTone + macroA * 0.85f);
        const float fmIndex = (0.10f + dynamicFm * 12.0f) * (1.0f + macroA * 0.55f);

        const float mod = std::sin (modPhase) * fmIndex * juce::jlimit (0.15f, 1.0f, ampEnv + 0.15f);
        const float oscNoise = rng.nextFloat() * 2.0f - 1.0f;
        const float carrier = renderCarrier (phase + mod, p.waveShape, oscNoise);

        const float noise = (rng.nextFloat() * 2.0f - 1.0f) * filterEnv * (0.03f + dynamicNoise * 0.55f);
        const float transientClick = (rng.nextFloat() * 2.0f - 1.0f)
            * (filterEnv * filterEnv) * (0.08f + dynamicNoise * 0.18f);

        float x = carrier * (0.70f + 0.48f * dynamicTone) + noise + transientClick;
        x = softFold (x * (1.12f + dynamicFm * 1.8f + macro * 0.55f + p.drive * 0.8f));

        phase += juce::MathConstants<float>::twoPi * freq / (float) sr;
        modPhase += juce::MathConstants<float>::twoPi * modFreq / (float) sr;
        if (phase > juce::MathConstants<float>::twoPi) phase -= juce::MathConstants<float>::twoPi;
        if (modPhase > juce::MathConstants<float>::twoPi) modPhase -= juce::MathConstants<float>::twoPi;

        float dynamicCutoff = p.filterBase;
        dynamicCutoff += filterEnv * (4500.0f + 11000.0f * p.filterBlast + 3200.0f * dynamicTone);
        dynamicCutoff *= std::pow (2.0f, macroB * 2.35f);
        dynamicCutoff = juce::jlimit (60.0f, 19000.0f, dynamicCutoff);

        const float dynamicRes = juce::jlimit (0.05f, 0.985f,
            p.resonance + macroA * 0.36f + chaos * 0.08f * randomOffset);
        const float dynamicDrive = juce::jlimit (0.0f, 1.0f, p.drive + macroB * 0.42f);

        // Nonlinear 4-pole cascade with resonant feedback for plasma / blaster sweeps.
        const float g = juce::jlimit (0.001f, 0.985f,
            1.0f - std::exp (-juce::MathConstants<float>::twoPi * dynamicCutoff / (float) sr));
        const float feedback = dynamicRes * 3.75f;
        const float driven = fastTanh (x * (1.0f + dynamicDrive * 4.8f) - feedback * f4);

        f1 += g * (driven - f1);
        f2 += g * (fastTanh (f1 * 1.15f) - f2);
        f3 += g * (fastTanh (f2 * 1.12f) - f3);
        f4 += g * (fastTanh (f3 * 1.10f) - f4);

        const float low = f4;
        const float band = (f2 - f4) * 1.8f;
        const float high = driven - f1 * 1.35f + f4 * 0.35f;

        float filterOut = low;
        if (p.filterType == 1) filterOut = band;
        else if (p.filterType == 2) filterOut = high;

        float filtered = fastTanh (filterOut * (1.0f + dynamicRes * 1.6f));
        filtered *= ampEnv * vel * dynamicLevel * 0.46f;

        const float panMotion = 0.28f * macroA + 0.10f * randomOffset;
        const float pan = juce::jlimit (-0.85f, 0.85f, ((voiceIndex - 1.5f) * 0.16f) + panMotion);
        const float lg = std::sqrt (0.5f * (1.0f - pan));
        const float rg = std::sqrt (0.5f * (1.0f + pan));

        left[startSample + n] += filtered * lg;
        right[startSample + n] += filtered * rg;

        const float ampCoeff = std::exp (-1.0f / (float) (sr * baseDecaySeconds));
        const float filterCoeff = std::exp (-1.0f / (float) (sr * juce::jmap (p.decay, 0.02f, 0.75f)));

        ampEnv *= ampCoeff;
        filterEnv *= filterCoeff;

        if (ampEnv < 0.00008f)
            active = false;
    }
}

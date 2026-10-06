#include "PlasmaVoice.h"

float PlasmaVoice::softFold (float x) noexcept
{
    x = std::fmod (x + 3.0f, 4.0f) - 2.0f;
    if (x > 1.0f) x = 2.0f - x;
    if (x < -1.0f) x = -2.0f - x;
    return fastTanh (x * 1.35f);
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
    if (! active)
        return;

    auto* left = output.getWritePointer (0);
    auto* right = output.getNumChannels() > 1 ? output.getWritePointer (1) : left;

    const float chaos = juce::jlimit (0.0f, 1.0f, globalChaos + p.chaos * 0.45f);
    const float macro = juce::jlimit (0.0f, 1.0f, p.everything);
    const float baseDecaySeconds = juce::jmap (p.decay, 0.03f, 1.8f);

    const float macroHzA = juce::jlimit (0.05f, 14.0f, p.syncHz * (0.5f + macro * 1.8f));
    const float macroHzB = juce::jlimit (0.07f, 18.0f, p.syncHz * (1.0f + macro * 2.2f));
    const float macroIncA = juce::MathConstants<float>::twoPi * macroHzA / (float) sr;
    const float macroIncB = juce::MathConstants<float>::twoPi * macroHzB / (float) sr;

    for (int n = 0; n < numSamples; ++n)
    {
        const float macroA = std::sin (macroPhaseA);
        const float macroB = std::sin (macroPhaseB + macroA * (0.6f + chaos));
        macroPhaseA += macroIncA;
        macroPhaseB += macroIncB;
        if (macroPhaseA > juce::MathConstants<float>::twoPi) macroPhaseA -= juce::MathConstants<float>::twoPi;
        if (macroPhaseB > juce::MathConstants<float>::twoPi) macroPhaseB -= juce::MathConstants<float>::twoPi;

        const float envPitchOct = (2.1f + 2.2f * p.envAmount + p.filterBlast * 0.8f) * filterEnv
                                  + macro * 0.45f * macroA;
        const float base = p.pitchHz * (1.0f + pitchJitter + macro * 0.05f * macroB);
        const float freq = juce::jlimit (20.0f, 12000.0f, base * std::pow (2.0f, envPitchOct));
        const float modFreq = freq * (1.2f + 2.4f * p.tone + 0.6f * macro * macroB);
        const float fmIndex = (0.12f + p.fm * 11.5f) * (1.0f + macro * 0.8f * macroA);

        const float mod = std::sin (modPhase) * fmIndex * juce::jlimit (0.15f, 1.0f, ampEnv + 0.15f);
        const float carrier = std::sin (phase + mod);

        const float dynamicNoise = juce::jlimit (0.0f, 1.0f, p.noise + macro * 0.28f * macroA);
        const float noise = (rng.nextFloat() * 2.0f - 1.0f) * filterEnv * (0.03f + dynamicNoise * 0.55f);
        const float transientClick = (rng.nextFloat() * 2.0f - 1.0f)
            * (filterEnv * filterEnv) * (0.08f + dynamicNoise * 0.18f);

        float x = carrier * (0.70f + 0.48f * p.tone) + noise + transientClick;
        x = softFold (x * (1.15f + p.fm * 1.6f + macro * 0.95f + p.drive * 0.8f));

        phase += juce::MathConstants<float>::twoPi * freq / (float) sr;
        modPhase += juce::MathConstants<float>::twoPi * modFreq / (float) sr;
        if (phase > juce::MathConstants<float>::twoPi) phase -= juce::MathConstants<float>::twoPi;
        if (modPhase > juce::MathConstants<float>::twoPi) modPhase -= juce::MathConstants<float>::twoPi;

        float dynamicCutoff = p.filterBase;
        dynamicCutoff += filterEnv * (4500.0f + 11000.0f * p.filterBlast + 3200.0f * p.tone);
        dynamicCutoff *= std::pow (2.0f, macro * 1.6f * macroB);
        dynamicCutoff = juce::jlimit (60.0f, 19000.0f, dynamicCutoff);

        const float dynamicRes = juce::jlimit (0.05f, 0.985f,
            p.resonance + macro * 0.22f * macroA + chaos * 0.08f * randomOffset);
        const float dynamicDrive = juce::jlimit (0.0f, 1.0f, p.drive + macro * 0.22f * macroB);

        // 4-pole nonlinear cascade. Resonant feedback plus the fast envelope
        // creates the "blaster / plasma" chirp.
        const float g = juce::jlimit (0.001f, 0.985f,
            1.0f - std::exp (-juce::MathConstants<float>::twoPi * dynamicCutoff / (float) sr));
        const float feedback = dynamicRes * 3.75f;
        const float driven = fastTanh (x * (1.0f + dynamicDrive * 4.8f) - feedback * f4);

        f1 += g * (driven - f1);
        f2 += g * (fastTanh (f1 * 1.15f) - f2);
        f3 += g * (fastTanh (f2 * 1.12f) - f3);
        f4 += g * (fastTanh (f3 * 1.10f) - f4);

        float filtered = fastTanh (f4 * (1.0f + dynamicRes * 1.6f));
        filtered *= ampEnv * vel * 0.40f;

        const float panMotion = 0.30f * macro * macroA + 0.10f * randomOffset;
        const float pan = juce::jlimit (-0.85f, 0.85f, ((voiceIndex - 1.5f) * 0.16f) + panMotion);
        const float lg = std::sqrt (0.5f * (1.0f - pan));
        const float rg = std::sqrt (0.5f * (1.0f + pan));

        left[startSample + n] += filtered * lg;
        right[startSample + n] += filtered * rg;

        const float dynamicDecaySeconds = juce::jlimit (0.02f, 2.2f,
            baseDecaySeconds * std::pow (2.0f, macro * 0.65f * macroB));
        const float ampCoeff = std::exp (-1.0f / (float) (sr * dynamicDecaySeconds));
        const float dynamicFilterDecay = juce::jlimit (0.015f, 1.2f,
            juce::jmap (p.decay, 0.02f, 0.75f) * std::pow (2.0f, macro * 0.45f * macroA));
        const float filterCoeff = std::exp (-1.0f / (float) (sr * dynamicFilterDecay));

        ampEnv *= ampCoeff;
        filterEnv *= filterCoeff;

        if (ampEnv < 0.00008f)
        {
            active = false;
            break;
        }
    }
}

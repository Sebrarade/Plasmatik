#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String pid (int v, const char* name)
{
    return "v" + juce::String (v + 1) + "_" + name;
}

PlasmaPercAudioProcessor::PlasmaPercAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for (auto& r : evolveRequests)
        r.store (false);
}

juce::AudioProcessorValueTreeState::ParameterLayout PlasmaPercAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const juce::StringArray divisions { "1/4", "1/8", "1/16", "1/32" };
    const juce::StringArray waves { "Sine", "Triangle", "Square", "Saw", "Noise" };
    const juce::StringArray lfoShapes { "Sine", "Triangle", "Saw", "Square", "S&H", "Smooth Random" };
    const juce::StringArray lfoRates { "4 Bars", "2 Bars", "1 Bar", "1/2", "1/4", "1/8", "1/16", "1/32" };

    for (int v = 0; v < 4; ++v)
    {
        const auto prefix = "V" + juce::String (v + 1) + " ";
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "pitch"), prefix + "Pitch",
                     juce::NormalisableRange<float> (45.0f, 1200.0f, 0.01f, 0.35f), 120.0f + v * 50.0f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "tone"), prefix + "Tone", 0.0f, 1.0f, 0.50f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "fm"), prefix + "FM", 0.0f, 1.0f, 0.36f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "noise"), prefix + "Noise", 0.0f, 1.0f, 0.24f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "level"), prefix + "Level", 0.0f, 1.0f, 0.82f));
        layout.add (std::make_unique<juce::AudioParameterChoice> (pid(v, "wave"), prefix + "Wave", waves, v % waves.size()));

        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "density"), prefix + "Density", 0.0f, 1.0f, 0.28f + v * 0.08f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "prob"), prefix + "Probability", 0.0f, 1.0f, 0.88f));
        layout.add (std::make_unique<juce::AudioParameterInt>   (pid(v, "rotate"), prefix + "Rotate", 0, 15, v * 2));
        layout.add (std::make_unique<juce::AudioParameterChoice>(pid(v, "division"), prefix + "Division", divisions, juce::jlimit (0, 3, v > 1 ? 2 : 1)));
        layout.add (std::make_unique<juce::AudioParameterBool> (pid(v, "drift"), prefix + "Drift", false));
        layout.add (std::make_unique<juce::AudioParameterBool> (pid(v, "auto"), prefix + "Auto", false));
        layout.add (std::make_unique<juce::AudioParameterBool> (pid(v, "mute"), prefix + "Mute", false));

        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "everything"), prefix + "Everything Amount", 0.0f, 1.0f, 0.34f));
        layout.add (std::make_unique<juce::AudioParameterChoice> (pid(v, "everything_shape"), prefix + "Everything Shape", lfoShapes, v == 3 ? 4 : v));
        layout.add (std::make_unique<juce::AudioParameterChoice> (pid(v, "everything_rate"), prefix + "Everything Rate", lfoRates, 5));
    }

    layout.add (std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", 0.0f, 1.0f, 0.30f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("env_amt", "Env Amount", 0.0f, 1.0f, 0.72f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("filter_base", "Filter Base",
                 juce::NormalisableRange<float> (80.0f, 12000.0f, 0.1f, 0.28f), 1650.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("filter_blast", "Filter Blast", 0.0f, 1.0f, 0.78f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("res", "Resonance", 0.0f, 1.0f, 0.70f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("drive", "Drive", 0.0f, 1.0f, 0.48f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("random", "Random", 0.0f, 1.0f, 0.22f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("variation", "Variation", 0.0f, 1.0f, 0.28f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("interact", "Interact", 0.0f, 1.0f, 0.36f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("chaos", "Chaos", 0.0f, 1.0f, 0.30f));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("crush", "Crush", 0.0f, 1.0f, 0.18f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("bits", "Bits", juce::NormalisableRange<float> (2.0f, 16.0f, 1.0f), 12.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("crush_mix", "Crusher Mix", 0.0f, 1.0f, 0.28f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("crush_lfo_depth", "Crusher LFO", 0.0f, 1.0f, 0.35f));
    layout.add (std::make_unique<juce::AudioParameterChoice> ("crush_lfo_shape", "Crusher LFO Shape", lfoShapes, 0));
    layout.add (std::make_unique<juce::AudioParameterChoice> ("crush_lfo_rate", "Crusher LFO Rate", lfoRates, 5));

    layout.add (std::make_unique<juce::AudioParameterFloat> ("width", "Width", 0.0f, 2.0f, 1.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("output", "Output", -24.0f, 6.0f, -6.0f));
    return layout;
}

float PlasmaPercAudioProcessor::syncedRateHz (int choice, double bpm) noexcept
{
    const double beatsPerSecond = juce::jmax (20.0, bpm) / 60.0;
    static constexpr double beatsPerCycle[] { 16.0, 8.0, 4.0, 2.0, 1.0, 0.5, 0.25, 0.125 };
    const int i = juce::jlimit (0, 7, choice);
    return (float) (beatsPerSecond / beatsPerCycle[i]);
}

void PlasmaPercAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    masterPhaseSamples = 0.0;
    master32Step = 0;
    localSteps = { 0, 0, 0, 0 };
    cycleCounts = { 0, 0, 0, 0 };
    previousHits.fill (false);
    lastDensity = { -1.0f, -1.0f, -1.0f, -1.0f };

    crusherPhase = 0.0f;
    crusherHoldValue = 0.0f;
    crusherSmoothFrom = 0.0f;
    crusherSmoothTo = 0.0f;
    crusherSampleCounter = 0;
    crusherHeldL = crusherHeldR = 0.0f;

    for (auto& v : voices)
        v.prepare (sampleRate, samplesPerBlock);

    for (int v = 0; v < 4; ++v)
    {
        const float density = apvts.getRawParameterValue (pid(v, "density"))->load();
        regeneratePattern (v, density, 0.18f);
        lastDensity[(size_t) v] = density;
    }
}

bool PlasmaPercAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

int PlasmaPercAudioProcessor::divisionInterval (int choice) noexcept
{
    switch (choice)
    {
        case 0: return 8;
        case 1: return 4;
        case 2: return 2;
        default: return 1;
    }
}

PlasmaVoiceParams PlasmaPercAudioProcessor::readVoiceParams (int i, double bpm) const
{
    PlasmaVoiceParams p;
    p.pitchHz = apvts.getRawParameterValue (pid(i, "pitch"))->load();
    p.tone = apvts.getRawParameterValue (pid(i, "tone"))->load();
    p.fm = apvts.getRawParameterValue (pid(i, "fm"))->load();
    p.noise = apvts.getRawParameterValue (pid(i, "noise"))->load();
    p.level = apvts.getRawParameterValue (pid(i, "level"))->load();
    p.waveShape = (int) apvts.getRawParameterValue (pid(i, "wave"))->load();

    p.decay = apvts.getRawParameterValue ("decay")->load();
    p.envAmount = apvts.getRawParameterValue ("env_amt")->load();
    p.filterBase = apvts.getRawParameterValue ("filter_base")->load();
    p.filterBlast = apvts.getRawParameterValue ("filter_blast")->load();
    p.resonance = apvts.getRawParameterValue ("res")->load();
    p.drive = apvts.getRawParameterValue ("drive")->load();
    p.chaos = apvts.getRawParameterValue ("chaos")->load();

    p.everythingAmount = apvts.getRawParameterValue (pid(i, "everything"))->load();
    p.everythingShape = (int) apvts.getRawParameterValue (pid(i, "everything_shape"))->load();
    p.everythingRateHz = syncedRateHz ((int) apvts.getRawParameterValue (pid(i, "everything_rate"))->load(), bpm);
    return p;
}

void PlasmaPercAudioProcessor::regeneratePattern (int voice, float density, float randomness)
{
    const int pulses = juce::jlimit (0, 16, (int) std::round (juce::jlimit (0.0f, 1.0f, density) * 16.0f));
    const int rotation = (int) apvts.getRawParameterValue (pid(voice, "rotate"))->load();
    const auto base = plasma::euclideanPattern (pulses, rotation);

    for (int s = 0; s < 16; ++s)
    {
        bool hit = base[(size_t) s];
        if (rng.nextFloat() < randomness * 0.26f)
            hit = ! hit;

        float p = hit ? (0.78f + 0.20f * rng.nextFloat()) : (0.015f + randomness * 0.15f * rng.nextFloat());
        patternProb[(size_t) voice][(size_t) s] = juce::jlimit (0.0f, 1.0f, p);
        patternDisplay[(size_t) voice][(size_t) s].store (patternProb[(size_t) voice][(size_t) s]);
    }
}

void PlasmaPercAudioProcessor::driftPattern (int voice, float amount)
{
    const int changes = 1 + (rng.nextFloat() < amount ? 1 : 0);
    for (int i = 0; i < changes; ++i)
    {
        const int s = rng.nextInt (16);
        float& p = patternProb[(size_t) voice][(size_t) s];
        p = juce::jlimit (0.0f, 1.0f, p + (rng.nextFloat() * 2.0f - 1.0f) * (0.10f + amount * 0.25f));
        patternDisplay[(size_t) voice][(size_t) s].store (p);
    }
}

void PlasmaPercAudioProcessor::processPatternRequests()
{
    const float variation = apvts.getRawParameterValue ("variation")->load();
    for (int v = 0; v < 4; ++v)
    {
        const float density = apvts.getRawParameterValue (pid(v, "density"))->load();

        if (std::abs (density - lastDensity[(size_t) v]) > 0.035f)
        {
            regeneratePattern (v, density, 0.18f + variation * 0.18f);
            lastDensity[(size_t) v] = density;
        }

        if (evolveRequests[(size_t) v].exchange (false))
            regeneratePattern (v, density, 0.48f + variation * 0.35f);
    }
}

bool PlasmaPercAudioProcessor::shouldTriggerVoice (int voice, int step, float interactBoost)
{
    const float prob = apvts.getRawParameterValue (pid(voice, "prob"))->load();
    const float globalRandom = apvts.getRawParameterValue ("random")->load();

    float chance = patternProb[(size_t) voice][(size_t) (step % 16)] * prob;
    chance += interactBoost;
    chance += (rng.nextFloat() * 2.0f - 1.0f) * globalRandom * 0.08f;
    return rng.nextFloat() < juce::jlimit (0.0f, 1.0f, chance);
}

void PlasmaPercAudioProcessor::requestEvolve (int voice)
{
    if (voice >= 0 && voice < 4)
        evolveRequests[(size_t) voice].store (true);
}

float PlasmaPercAudioProcessor::getPatternValue (int voice, int step) const noexcept
{
    if (voice < 0 || voice >= 4 || step < 0 || step >= 16)
        return 0.0f;
    return patternDisplay[(size_t) voice][(size_t) step].load();
}

float PlasmaPercAudioProcessor::nextCrusherLfo (int shape, float rateHz)
{
    const float inc = juce::jmax (0.0000001f, rateHz / (float) currentSampleRate);
    crusherPhase += inc;
    if (crusherPhase >= 1.0f)
    {
        crusherPhase -= std::floor (crusherPhase);
        crusherHoldValue = rng.nextFloat() * 2.0f - 1.0f;
        crusherSmoothFrom = crusherSmoothTo;
        crusherSmoothTo = rng.nextFloat() * 2.0f - 1.0f;
    }

    const float p = crusherPhase;
    switch (shape)
    {
        case 1: return 1.0f - 4.0f * std::abs (p - 0.5f);
        case 2: return 2.0f * p - 1.0f;
        case 3: return p < 0.5f ? 1.0f : -1.0f;
        case 4: return crusherHoldValue;
        case 5:
        {
            const float t = p * p * (3.0f - 2.0f * p);
            return crusherSmoothFrom + (crusherSmoothTo - crusherSmoothFrom) * t;
        }
        default: return std::sin (juce::MathConstants<float>::twoPi * p);
    }
}

void PlasmaPercAudioProcessor::processBitcrusher (juce::AudioBuffer<float>& buffer, double bpm)
{
    if (buffer.getNumChannels() < 1)
        return;

    const float crush = apvts.getRawParameterValue ("crush")->load();
    const float bits = apvts.getRawParameterValue ("bits")->load();
    const float mix = apvts.getRawParameterValue ("crush_mix")->load();
    const float lfoDepth = apvts.getRawParameterValue ("crush_lfo_depth")->load();
    const int shape = (int) apvts.getRawParameterValue ("crush_lfo_shape")->load();
    const int rate = (int) apvts.getRawParameterValue ("crush_lfo_rate")->load();
    const float rateHz = syncedRateHz (rate, bpm);

    auto* l = buffer.getWritePointer (0);
    auto* r = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    const float levels = std::pow (2.0f, juce::jlimit (2.0f, 16.0f, bits) - 1.0f);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        const float lfo = nextCrusherLfo (shape, rateHz);
        const float effectiveCrush = juce::jlimit (0.0f, 1.0f, crush + lfo * lfoDepth * 0.48f);
        const int hold = 1 + (int) std::round (std::pow (effectiveCrush, 2.1f) * 63.0f);

        if (crusherSampleCounter <= 0)
        {
            crusherHeldL = std::round (l[i] * levels) / levels;
            if (r != nullptr)
                crusherHeldR = std::round (r[i] * levels) / levels;
            crusherSampleCounter = hold;
        }

        --crusherSampleCounter;

        const float dryL = l[i];
        l[i] = dryL + (crusherHeldL - dryL) * mix;

        if (r != nullptr)
        {
            const float dryR = r[i];
            r[i] = dryR + (crusherHeldR - dryR) * mix;
        }
    }
}

void PlasmaPercAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    processPatternRequests();

    double bpm = 120.0;
    bool playing = true;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            playing = pos->getIsPlaying();
        }
    }

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            const int idx = msg.getNoteNumber() - 36;
            if (idx >= 0 && idx < 4 && apvts.getRawParameterValue (pid(idx, "mute"))->load() < 0.5f)
                voices[(size_t) idx].trigger (msg.getFloatVelocity(), readVoiceParams (idx, bpm),
                    apvts.getRawParameterValue ("chaos")->load(), triggerCounter++ * 2654435761u);
        }
    }

    const double samplesPerMasterStep = currentSampleRate * 60.0 / juce::jmax (20.0, bpm) / 8.0;

    int cursor = 0;
    while (cursor < buffer.getNumSamples())
    {
        int chunk = buffer.getNumSamples() - cursor;
        if (playing)
        {
            const double untilNext = samplesPerMasterStep - masterPhaseSamples;
            chunk = juce::jlimit (1, buffer.getNumSamples() - cursor, (int) std::ceil (untilNext));
        }

        for (int v = 0; v < 4; ++v)
        {
            auto params = readVoiceParams (v, bpm);
            const bool muted = apvts.getRawParameterValue (pid(v, "mute"))->load() >= 0.5f;
            if (! muted)
                voices[(size_t) v].process (buffer, cursor, chunk, params, apvts.getRawParameterValue ("chaos")->load(), v);
        }

        cursor += chunk;

        if (! playing)
            continue;

        masterPhaseSamples += chunk;
        if (masterPhaseSamples >= samplesPerMasterStep)
        {
            masterPhaseSamples -= samplesPerMasterStep;
            master32Step = (master32Step + 1) % 128;

            const float interact = apvts.getRawParameterValue ("interact")->load();
            const float variation = apvts.getRawParameterValue ("variation")->load();
            std::array<bool, 4> hits { false, false, false, false };

            for (int v = 0; v < 4; ++v)
            {
                if (apvts.getRawParameterValue (pid(v, "mute"))->load() >= 0.5f)
                    continue;

                const int divChoice = (int) apvts.getRawParameterValue (pid(v, "division"))->load();
                const int interval = divisionInterval (divChoice);
                if ((master32Step % interval) != 0)
                    continue;

                const int leftNeighbour = (v + 3) % 4;
                const float boost = previousHits[(size_t) leftNeighbour] ? interact * 0.26f : 0.0f;
                const bool hit = shouldTriggerVoice (v, localSteps[(size_t) v], boost);
                hits[(size_t) v] = hit;

                if (hit)
                {
                    float velocity = 0.58f + rng.nextFloat() * 0.38f;
                    if (previousHits[(size_t) leftNeighbour])
                        velocity = juce::jlimit (0.0f, 1.0f, velocity + interact * 0.16f);

                    voices[(size_t) v].trigger (velocity, readVoiceParams (v, bpm),
                        apvts.getRawParameterValue ("chaos")->load(), triggerCounter++ * 2654435761u);
                }

                localSteps[(size_t) v] = (localSteps[(size_t) v] + 1) % 16;
                if (localSteps[(size_t) v] == 0)
                {
                    ++cycleCounts[(size_t) v];

                    if (apvts.getRawParameterValue (pid(v, "drift"))->load() >= 0.5f)
                        driftPattern (v, 0.12f + variation * 0.35f);

                    if (apvts.getRawParameterValue (pid(v, "auto"))->load() >= 0.5f)
                    {
                        const float density = apvts.getRawParameterValue (pid(v, "density"))->load();
                        regeneratePattern (v, density, 0.32f + variation * 0.48f);
                    }
                }
            }

            previousHits = hits;
        }
    }

    processBitcrusher (buffer, bpm);

    if (buffer.getNumChannels() > 1)
    {
        const float width = apvts.getRawParameterValue ("width")->load();
        auto* l = buffer.getWritePointer (0);
        auto* r = buffer.getWritePointer (1);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            const float mid = 0.5f * (l[i] + r[i]);
            const float side = 0.5f * (l[i] - r[i]) * width;
            l[i] = mid + side;
            r[i] = mid - side;
        }
    }

    buffer.applyGain (juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("output")->load()));
}

void PlasmaPercAudioProcessor::mutate (float amount)
{
    amount = juce::jlimit (0.0f, 1.0f, amount);

    auto setNorm = [this, amount] (const juce::String& id, float target)
    {
        if (auto* p = apvts.getParameter (id))
        {
            const float old = p->getValue();
            p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, old + (target - old) * amount));
        }
    };

    for (int v = 0; v < 4; ++v)
    {
        setNorm (pid(v, "pitch"), 0.08f + 0.80f * rng.nextFloat());
        setNorm (pid(v, "tone"), rng.nextFloat());
        setNorm (pid(v, "fm"), 0.10f + 0.85f * rng.nextFloat());
        setNorm (pid(v, "noise"), 0.02f + 0.75f * rng.nextFloat());
        setNorm (pid(v, "level"), 0.45f + 0.52f * rng.nextFloat());
        setNorm (pid(v, "wave"), rng.nextFloat());
        setNorm (pid(v, "density"), 0.08f + 0.78f * rng.nextFloat());
        setNorm (pid(v, "prob"), 0.48f + 0.50f * rng.nextFloat());
        setNorm (pid(v, "everything"), 0.08f + 0.85f * rng.nextFloat());
        setNorm (pid(v, "everything_shape"), rng.nextFloat());
        setNorm (pid(v, "everything_rate"), rng.nextFloat());
        setNorm (pid(v, "rotate"), rng.nextFloat());
        setNorm (pid(v, "division"), rng.nextFloat());
        requestEvolve (v);
    }

    // Decay is intentionally excluded from MUTATE.
    setNorm ("env_amt", 0.25f + 0.70f * rng.nextFloat());
    setNorm ("filter_base", 0.08f + 0.60f * rng.nextFloat());
    setNorm ("filter_blast", 0.20f + 0.78f * rng.nextFloat());
    setNorm ("res", 0.42f + 0.52f * rng.nextFloat());
    setNorm ("drive", 0.18f + 0.68f * rng.nextFloat());
    setNorm ("random", 0.05f + 0.72f * rng.nextFloat());
    setNorm ("variation", 0.05f + 0.78f * rng.nextFloat());
    setNorm ("interact", 0.05f + 0.80f * rng.nextFloat());
    setNorm ("chaos", 0.05f + 0.78f * rng.nextFloat());

    setNorm ("crush", 0.02f + 0.82f * rng.nextFloat());
    setNorm ("bits", 0.25f + 0.72f * rng.nextFloat());
    setNorm ("crush_mix", 0.05f + 0.75f * rng.nextFloat());
    setNorm ("crush_lfo_depth", 0.02f + 0.78f * rng.nextFloat());
    setNorm ("crush_lfo_shape", rng.nextFloat());
    setNorm ("crush_lfo_rate", rng.nextFloat());
}

void PlasmaPercAudioProcessor::loadPreset (int presetIndex)
{
    auto setActual = [this] (const juce::String& id, float value)
    {
        if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (id)))
            p->setValueNotifyingHost (p->convertTo0to1 (value));
    };

    if (presetIndex == 0)
    {
        setActual ("decay", 0.28f); setActual ("filter_blast", 0.78f); setActual ("res", 0.70f); setActual ("drive", 0.45f);
        setActual ("crush", 0.16f); setActual ("crush_mix", 0.22f);
    }
    else if (presetIndex == 1)
    {
        setActual ("decay", 0.16f); setActual ("filter_blast", 0.92f); setActual ("res", 0.82f); setActual ("drive", 0.62f);
        setActual ("crush", 0.42f); setActual ("crush_mix", 0.48f);
    }
    else if (presetIndex == 2)
    {
        setActual ("decay", 0.48f); setActual ("filter_blast", 0.48f); setActual ("res", 0.56f); setActual ("drive", 0.72f);
        setActual ("crush", 0.62f); setActual ("crush_mix", 0.68f);
    }
    else
    {
        setActual ("decay", 0.34f); setActual ("filter_blast", 0.86f); setActual ("res", 0.76f); setActual ("drive", 0.38f);
        setActual ("crush", 0.25f); setActual ("crush_mix", 0.34f);
    }

    for (int v = 0; v < 4; ++v)
        requestEvolve (v);
}

void PlasmaPercAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PlasmaPercAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            lastDensity = { -1.0f, -1.0f, -1.0f, -1.0f };
            for (int v = 0; v < 4; ++v)
                requestEvolve (v);
        }
    }
}

juce::AudioProcessorEditor* PlasmaPercAudioProcessor::createEditor()
{
    return new PlasmaPercAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PlasmaPercAudioProcessor();
}

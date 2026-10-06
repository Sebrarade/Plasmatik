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
}

juce::AudioProcessorValueTreeState::ParameterLayout PlasmaPercAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int v = 0; v < 4; ++v)
    {
        const auto prefix = "V" + juce::String (v + 1) + " ";
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "pitch"), prefix + "Pitch",
                     juce::NormalisableRange<float> (45.0f, 1200.0f, 0.01f, 0.35f), 120.0f + v * 50.0f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "tone"), prefix + "Tone", 0.0f, 1.0f, 0.50f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "fm"), prefix + "FM", 0.0f, 1.0f, 0.36f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "noise"), prefix + "Noise", 0.0f, 1.0f, 0.24f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "density"), prefix + "Density", 0.0f, 1.0f, 0.28f + v * 0.08f));
        layout.add (std::make_unique<juce::AudioParameterInt>   (pid(v, "rotate"), prefix + "Rotate", 0, 15, v * 2));
        layout.add (std::make_unique<juce::AudioParameterChoice>(pid(v, "division"), prefix + "Division",
                     juce::StringArray { "1/4", "1/8", "1/16", "1/32" }, juce::jlimit (0, 3, v > 1 ? 2 : 1)));
        layout.add (std::make_unique<juce::AudioParameterFloat> (pid(v, "everything"), prefix + "Everything", 0.0f, 1.0f, 0.34f));
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
    layout.add (std::make_unique<juce::AudioParameterFloat> ("output", "Output", -24.0f, 6.0f, -6.0f));
    return layout;
}

void PlasmaPercAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    masterPhaseSamples = 0.0;
    master32Step = 0;
    localSteps = { 0, 0, 0, 0 };
    cycleCounts = { 0, 0, 0, 0 };
    previousHits.fill (false);
    for (auto& v : voices)
        v.prepare (sampleRate, samplesPerBlock);
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
    p.everything = apvts.getRawParameterValue (pid(i, "everything"))->load();
    p.decay = apvts.getRawParameterValue ("decay")->load();
    p.envAmount = apvts.getRawParameterValue ("env_amt")->load();
    p.filterBase = apvts.getRawParameterValue ("filter_base")->load();
    p.filterBlast = apvts.getRawParameterValue ("filter_blast")->load();
    p.resonance = apvts.getRawParameterValue ("res")->load();
    p.drive = apvts.getRawParameterValue ("drive")->load();
    p.chaos = apvts.getRawParameterValue ("chaos")->load();
    const int div = (int) apvts.getRawParameterValue (pid(i, "division"))->load();
    const float mult = div == 0 ? 1.0f : (div == 1 ? 2.0f : (div == 2 ? 4.0f : 8.0f));
    p.syncHz = juce::jmax (0.05f, (float) (bpm / 60.0) * mult);
    return p;
}

bool PlasmaPercAudioProcessor::shouldTriggerVoice (int voice, int step, float interactBoost)
{
    const float everything = apvts.getRawParameterValue (pid(voice, "everything"))->load();
    const float densityBase = apvts.getRawParameterValue (pid(voice, "density"))->load();
    const float densityWobble = 0.14f * everything
        * std::sin ((float) cycleCounts[(size_t) voice] * 1.173f + voice * 0.77f);
    const float density = juce::jlimit (0.0f, 1.0f, densityBase + densityWobble);
    const int pulses = juce::jlimit (0, 16, (int) std::round (density * 16.0f));
    const int rotation = (int) apvts.getRawParameterValue (pid(voice, "rotate"))->load();
    const auto pattern = plasma::euclideanPattern (pulses, rotation);

    float randomAmt = apvts.getRawParameterValue ("random")->load();
    const float variation = apvts.getRawParameterValue ("variation")->load();
    randomAmt = juce::jlimit (0.0f, 1.0f, randomAmt
        + variation * 0.32f * std::sin ((float) cycleCounts[(size_t) voice] * 1.618f + voice * 0.9f)
        + everything * 0.20f);

    bool hit = pattern[(size_t) (step % 16)];
    if (rng.nextFloat() < randomAmt * 0.24f)
        hit = ! hit;

    if (! hit && rng.nextFloat() < interactBoost)
        hit = true;

    return hit;
}

void PlasmaPercAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

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
            if (idx >= 0 && idx < 4)
                voices[(size_t) idx].trigger (msg.getFloatVelocity(), readVoiceParams (idx, bpm),
                    apvts.getRawParameterValue ("chaos")->load(), triggerCounter++ * 2654435761u);
        }
    }

    const double samplesPerMasterStep = currentSampleRate * 60.0 / juce::jmax (20.0, bpm) / 8.0;

    if (playing)
    {
        int cursor = 0;
        while (cursor < buffer.getNumSamples())
        {
            const double untilNext = samplesPerMasterStep - masterPhaseSamples;
            const int chunk = juce::jlimit (1, buffer.getNumSamples() - cursor, (int) std::ceil (untilNext));

            for (int v = 0; v < 4; ++v)
                voices[(size_t) v].process (buffer, cursor, chunk, readVoiceParams (v, bpm),
                    apvts.getRawParameterValue ("chaos")->load(), v);

            cursor += chunk;
            masterPhaseSamples += chunk;

            if (masterPhaseSamples >= samplesPerMasterStep)
            {
                masterPhaseSamples -= samplesPerMasterStep;
                master32Step = (master32Step + 1) % 128;

                const float interact = apvts.getRawParameterValue ("interact")->load();
                std::array<bool, 4> hits { false, false, false, false };

                for (int v = 0; v < 4; ++v)
                {
                    const int divChoice = (int) apvts.getRawParameterValue (pid(v, "division"))->load();
                    const int interval = divisionInterval (divChoice);
                    if ((master32Step % interval) != 0)
                    {
                        hits[(size_t) v] = false;
                        continue;
                    }

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
                        ++cycleCounts[(size_t) v];
                }

                previousHits = hits;
            }
        }
    }
    else
    {
        for (int v = 0; v < 4; ++v)
            voices[(size_t) v].process (buffer, 0, buffer.getNumSamples(), readVoiceParams (v, bpm),
                apvts.getRawParameterValue ("chaos")->load(), v);
    }

    const float outGain = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("output")->load());
    buffer.applyGain (outGain);
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
        setNorm (pid(v, "density"), 0.08f + 0.78f * rng.nextFloat());
        setNorm (pid(v, "everything"), 0.08f + 0.85f * rng.nextFloat());
        if (auto* p = apvts.getParameter (pid(v, "rotate")))
            p->setValueNotifyingHost (rng.nextFloat());
        if (auto* p = apvts.getParameter (pid(v, "division")))
            p->setValueNotifyingHost (0.2f + 0.7f * rng.nextFloat());
    }

    setNorm ("decay", 0.10f + 0.56f * rng.nextFloat());
    setNorm ("env_amt", 0.25f + 0.70f * rng.nextFloat());
    setNorm ("filter_base", 0.08f + 0.60f * rng.nextFloat());
    setNorm ("filter_blast", 0.20f + 0.78f * rng.nextFloat());
    setNorm ("res", 0.42f + 0.52f * rng.nextFloat());
    setNorm ("drive", 0.18f + 0.68f * rng.nextFloat());
    setNorm ("random", 0.05f + 0.72f * rng.nextFloat());
    setNorm ("variation", 0.05f + 0.78f * rng.nextFloat());
    setNorm ("interact", 0.05f + 0.80f * rng.nextFloat());
    setNorm ("chaos", 0.05f + 0.78f * rng.nextFloat());
}

void PlasmaPercAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void PlasmaPercAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* PlasmaPercAudioProcessor::createEditor()
{
    return new PlasmaPercAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PlasmaPercAudioProcessor();
}

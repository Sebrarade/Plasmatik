#include "BinaryData.h"
#include "PluginEditor.h"

static juce::String epId (int v, const char* n)
{
    return "v" + juce::String (v + 1) + "_" + n;
}

PlasmaPercAudioProcessorEditor::PlasmaPercAudioProcessorEditor (PlasmaPercAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (1700, 540);

    subtitle.setText ("4-VOICE GENERATIVE PLASMA / LASER PERCUSSION", juce::dontSendNotification);
    subtitle.setJustificationType (juce::Justification::centred);
    subtitle.setFont (juce::Font (juce::FontOptions (15.0f)).boldened());
    addAndMakeVisible (subtitle);

    fairyImage = juce::ImageCache::getFromMemory (BinaryData::fairy_jpg, BinaryData::fairy_jpgSize);

    const std::array<const char*, 4> engineNames { "PITCH", "TONE", "FM", "NOISE" };
    const std::array<const char*, 4> engineIds { "pitch", "tone", "fm", "noise" };

    for (int v = 0; v < 4; ++v)
    {
        auto& seq = seqs[(size_t) v];
        seq.title.setText ("TRK " + juce::String (v + 1), juce::dontSendNotification);
        seq.title.setJustificationType (juce::Justification::centredLeft);
        seq.title.setFont (juce::Font (juce::FontOptions (14.0f)).boldened());
        addAndMakeVisible (seq.title);
        styleCombo (seq.division, seq.divisionL, "RATE");
        seq.division.addItemList ({ "1/4", "1/8", "1/16", "1/32" }, 1);
        seq.divA = std::make_unique<ComboAttachment> (processor.getAPVTS(), epId (v, "division"), seq.division);
        styleKnob (seq.density, seq.densityL, "DENS", false);
        styleKnob (seq.rotate, seq.rotateL, "ROT", false);
        seq.densityA = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "density"), seq.density);
        seq.rotateA = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "rotate"), seq.rotate);

        auto& e = engines[(size_t) v];
        e.title.setText ("VOICE " + juce::String (v + 1), juce::dontSendNotification);
        e.title.setJustificationType (juce::Justification::centred);
        e.title.setFont (juce::Font (juce::FontOptions (16.0f)).boldened());
        addAndMakeVisible (e.title);
        for (int k = 0; k < 4; ++k)
        {
            styleKnob (e.knobs[(size_t) k], e.labels[(size_t) k], engineNames[(size_t) k], false);
            e.attachments[(size_t) k] = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, engineIds[(size_t) k]), e.knobs[(size_t) k]);
        }

        auto& m = macros[(size_t) v];
        m.title.setText ("EVERYTHING " + juce::String (v + 1), juce::dontSendNotification);
        m.title.setJustificationType (juce::Justification::centred);
        m.title.setFont (juce::Font (juce::FontOptions (14.0f)).boldened());
        addAndMakeVisible (m.title);
        styleKnob (m.knob, m.label, "EVERYTHING", true);
        m.attachment = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "everything"), m.knob);
    }

    const std::array<const char*, 10> globalNames {
        "DECAY", "ENV", "BASE", "BLAST", "RES", "DRIVE", "RANDOM", "VAR", "INTERACT", "CHAOS"
    };
    const std::array<const char*, 10> globalIds {
        "decay", "env_amt", "filter_base", "filter_blast", "res", "drive", "random", "variation", "interact", "chaos"
    };
    for (int i = 0; i < 10; ++i)
    {
        styleKnob (globals[(size_t) i], globalLabels[(size_t) i], globalNames[(size_t) i], false);
        globalAttachments[(size_t) i] = std::make_unique<SliderAttachment> (processor.getAPVTS(), globalIds[(size_t) i], globals[(size_t) i]);
    }

    mutate.onClick = [this] { processor.mutate(); };
    mutate.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (85, 28, 124));
    mutate.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible (mutate);
}

void PlasmaPercAudioProcessorEditor::styleKnob (juce::Slider& s, juce::Label& l, const juce::String& name, bool large)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, large ? 74 : 58, 18);
    s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB (202, 80, 255));
    s.setColour (juce::Slider::thumbColourId, juce::Colour::fromRGB (220, 250, 255));
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (s);
    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (juce::FontOptions (11.0f)).boldened());
    addAndMakeVisible (l);
}

void PlasmaPercAudioProcessorEditor::styleCombo (juce::ComboBox& box, juce::Label& l, const juce::String& name)
{
    box.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (28, 20, 44));
    box.setColour (juce::ComboBox::textColourId, juce::Colours::white);
    box.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (130, 70, 190));
    addAndMakeVisible (box);
    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (juce::FontOptions (11.0f)).boldened());
    addAndMakeVisible (l);
}

void PlasmaPercAudioProcessorEditor::drawPanel (juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title)
{
    g.setColour (juce::Colour::fromRGB (14, 14, 22).withAlpha (0.74f));
    g.fillRoundedRectangle (r.toFloat(), 14.0f);
    g.setColour (juce::Colour::fromRGB (109, 65, 165).withAlpha (0.6f));
    g.drawRoundedRectangle (r.toFloat(), 14.0f, 1.5f);
    g.setColour (juce::Colour::fromRGB (226, 232, 240));
    g.setFont (juce::Font (juce::FontOptions (14.0f)).boldened());
    g.drawText (title, r.removeFromTop (24), juce::Justification::centredLeft);
}

void PlasmaPercAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (8, 9, 13));
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient grad (juce::Colour::fromRGB (30, 10, 52), bounds.getX(), 0,
                               juce::Colour::fromRGB (8, 20, 28), bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRect (bounds);

    g.setColour (juce::Colour::fromRGB (232, 239, 245));
    g.setFont (juce::Font (juce::FontOptions (32.0f)).boldened());
    g.drawText ("PLASMATIK", 0, 12, getWidth(), 36, juce::Justification::centred);

    auto area = getLocalBounds().reduced (14);
    auto topArea = area.withTrimmedTop (72).withHeight (430);

    auto seqArea = topArea.removeFromLeft (280).reduced (4);
    auto engineArea = topArea.removeFromLeft (760).reduced (4);
    auto globalArea = topArea.removeFromLeft (380).reduced (4);
    auto macroArea = topArea.reduced (4);

    drawPanel (g, seqArea, "SEQUENCERS");
    drawPanel (g, engineArea, "VOICE ENGINES");
    drawPanel (g, globalArea, "ENV / FILTER / GENERATIVE");
    drawPanel (g, macroArea, "EVERYTHING MACROS");

    if (fairyImage.isValid())
    {
        g.setOpacity (0.28f);
        auto imgArea = macroArea.removeFromBottom (210).reduced (18, 0).withTrimmedLeft (4);
        g.drawImageWithin (fairyImage, imgArea.getX(), imgArea.getY(), imgArea.getWidth(), imgArea.getHeight(), juce::RectanglePlacement::centred, false);
        g.setOpacity (1.0f);
    }
}

void PlasmaPercAudioProcessorEditor::resized()
{
    subtitle.setBounds (0, 48, getWidth(), 20);

    auto area = getLocalBounds().reduced (18);
    auto body = area.withTrimmedTop (76).withHeight (425);

    auto seqArea = body.removeFromLeft (280).reduced (12, 14);
    auto engineArea = body.removeFromLeft (760).reduced (12, 14);
    auto globalArea = body.removeFromLeft (380).reduced (12, 14);
    auto macroArea = body.reduced (12, 14);

    seqArea.removeFromTop (22);
    const int rowH = 88;
    for (int v = 0; v < 4; ++v)
    {
        auto r = seqArea.removeFromTop (rowH);
        auto& s = seqs[(size_t) v];
        s.title.setBounds (r.removeFromTop (20));
        auto line = r.reduced (0, 6);

        auto c0 = line.removeFromLeft (90);
        s.divisionL.setBounds (c0.removeFromTop (16));
        s.division.setBounds (c0.reduced (2, 6));

        auto c1 = line.removeFromLeft (84);
        s.densityL.setBounds (c1.removeFromTop (16));
        s.density.setBounds (c1);

        auto c2 = line.removeFromLeft (84);
        s.rotateL.setBounds (c2.removeFromTop (16));
        s.rotate.setBounds (c2);
    }

    engineArea.removeFromTop (20);
    const int engineW = engineArea.getWidth() / 4;
    for (int v = 0; v < 4; ++v)
    {
        auto col = engineArea.removeFromLeft (engineW).reduced (4, 2);
        auto& e = engines[(size_t) v];
        e.title.setBounds (col.removeFromTop (24));
        for (int k = 0; k < 4; ++k)
        {
            auto row = col.removeFromTop (84);
            e.labels[(size_t) k].setBounds (row.removeFromTop (16));
            e.knobs[(size_t) k].setBounds (row.withSizeKeepingCentre (78, 64));
            col.removeFromTop (2);
        }
    }

    globalArea.removeFromTop (20);
    const int gw = globalArea.getWidth() / 2;
    for (int i = 0; i < 10; ++i)
    {
        const int col = i % 2;
        const int row = i / 2;
        const int x = globalArea.getX() + col * gw;
        const int y = globalArea.getY() + row * 74;
        globalLabels[(size_t) i].setBounds (x + 8, y, gw - 16, 16);
        globals[(size_t) i].setBounds (x + 18, y + 14, 84, 58);
    }

    mutate.setBounds (globalArea.getX() + 204, globalArea.getBottom() - 62, 132, 40);

    auto macroWork = macroArea;
    macroWork.removeFromTop (20);
    const int mh = 82;
    for (int v = 0; v < 4; ++v)
    {
        auto r = macroWork.removeFromTop (mh);
        auto& m = macros[(size_t) v];
        m.title.setBounds (r.removeFromTop (18));
        m.label.setBounds (r.getX(), r.getY(), r.getWidth(), 16);
        m.knob.setBounds (r.withSizeKeepingCentre (112, 72));
    }
}

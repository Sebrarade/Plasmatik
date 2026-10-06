#include "BinaryData.h"
#include "PluginEditor.h"

static juce::String epId (int v, const char* n)
{
    return "v" + juce::String (v + 1) + "_" + n;
}

//==============================================================================
// Look & Feel

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::drawRotarySlider (
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (5.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();

    g.setColour (juce::Colour::fromRGB (229, 229, 226));
    g.fillEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

    g.setColour (juce::Colour::fromRGB (190, 191, 190));
    g.drawEllipse (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.0f);

    const float arcRadius = radius + 2.0f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour::fromRGB (205, 206, 204));
    g.strokePath (backgroundArc, juce::PathStrokeType (2.3f, juce::PathStrokeType::curved));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                            rotaryStartAngle,
                            rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle), true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
    g.strokePath (valueArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved));

    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto dot = centre + juce::Point<float> (std::sin (angle), -std::cos (angle)) * (radius * 0.55f);
    g.setColour (juce::Colour::fromRGB (22, 24, 27));
    g.fillEllipse (dot.x - 2.3f, dot.y - 2.3f, 4.6f, 4.6f);
}

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::drawButtonBackground (
    juce::Graphics& g, juce::Button& button, const juce::Colour&, bool isHighlighted, bool isDown)
{
    auto r = button.getLocalBounds().toFloat().reduced (0.5f);
    auto colour = button.getToggleState()
        ? button.findColour (juce::TextButton::buttonOnColourId)
        : button.findColour (juce::TextButton::buttonColourId);

    if (isHighlighted) colour = colour.contrasting (0.05f);
    if (isDown) colour = colour.darker (0.08f);

    g.setColour (colour);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (juce::Colour::fromRGB (196, 198, 198));
    g.drawRoundedRectangle (r, 5.0f, 1.0f);
}

//==============================================================================
// Pattern view

PlasmaPercAudioProcessorEditor::PatternView::PatternView (
    PlasmaPercAudioProcessor& p, int voiceIndex, juce::Colour colour)
    : processor (p), voice (voiceIndex), accent (colour)
{
    startTimerHz (20);
}

void PlasmaPercAudioProcessorEditor::PatternView::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat().reduced (2.0f);
    const float stepW = area.getWidth() / 16.0f;

    juce::Path curve;
    bool started = false;

    for (int s = 0; s < 16; ++s)
    {
        const float p = processor.getPatternValue (voice, s);
        const float h = juce::jmap (p, 0.0f, 1.0f, 3.0f, area.getHeight() * 0.80f);
        const float x = area.getX() + stepW * (float) s + stepW * 0.18f;
        const float w = stepW * 0.55f;
        const float y = area.getBottom() - h;

        g.setColour (accent.withAlpha (0.60f));
        g.fillRoundedRectangle (x, y, w, h, juce::jmin (2.0f, w * 0.3f));

        const auto point = juce::Point<float> (x + w * 0.5f, y - 3.0f);
        if (! started)
        {
            curve.startNewSubPath (point);
            started = true;
        }
        else
        {
            curve.lineTo (point);
        }

        g.setColour (accent.withAlpha (0.9f));
        g.fillEllipse (point.x - 1.7f, point.y - 1.7f, 3.4f, 3.4f);
    }

    g.setColour (accent.withAlpha (0.72f));
    g.strokePath (curve, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved));
}

//==============================================================================

PlasmaPercAudioProcessorEditor::PlasmaPercAudioProcessorEditor (PlasmaPercAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);
    setResizable (true, true);
    setResizeLimits (1120, 630, 1920, 1080);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio (16.0 / 9.0);
    setSize (1500, 844);

    fairyImage = juce::ImageCache::getFromMemory (BinaryData::fairy_jpg, BinaryData::fairy_jpgSize);

    subtitle.setText ("GENERATIVE PLASMA / LASER PERCUSSION", juce::dontSendNotification);
    subtitle.setJustificationType (juce::Justification::centred);
    subtitle.setColour (juce::Label::textColourId, juce::Colour::fromRGB (45, 47, 50));
    subtitle.setFont (juce::Font (juce::FontOptions (13.0f)).boldened());
    addAndMakeVisible (subtitle);

    preset.addItem ("Plasma Garden", 1);
    preset.addItem ("Laser Dust", 2);
    preset.addItem ("Broken Glass", 3);
    preset.addItem ("Soft Machine", 4);
    preset.setSelectedId (1, juce::dontSendNotification);
    preset.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (247, 247, 245));
    preset.setColour (juce::ComboBox::textColourId, juce::Colour::fromRGB (24, 26, 28));
    preset.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (199, 201, 201));
    preset.onChange = [this]
    {
        processor.loadPreset (juce::jmax (0, preset.getSelectedItemIndex()));
    };
    addAndMakeVisible (preset);

    const juce::StringArray lfoShapes { "Sine", "Triangle", "Saw", "Square", "S&H", "Smooth Random" };
    const juce::StringArray lfoRates { "4 Bars", "2 Bars", "1 Bar", "1/2", "1/4", "1/8", "1/16", "1/32" };
    const juce::StringArray waveNames { "Sine", "Triangle", "Square", "Saw", "Noise" };

    for (int v = 0; v < 4; ++v)
    {
        const auto accent = voiceColour (v);

        auto& seq = seqs[(size_t) v];
        seq.title.setText ("TRK " + juce::String (v + 1), juce::dontSendNotification);
        seq.title.setJustificationType (juce::Justification::centredLeft);
        seq.title.setColour (juce::Label::textColourId, juce::Colour::fromRGB (25, 27, 29));
        seq.title.setFont (juce::Font (juce::FontOptions (14.0f)).boldened());
        addAndMakeVisible (seq.title);

        seq.division.addItemList ({ "1/4", "1/8", "1/16", "1/32" }, 1);
        seq.division.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (249, 249, 247));
        seq.division.setColour (juce::ComboBox::textColourId, juce::Colour::fromRGB (24, 26, 28));
        seq.division.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (200, 202, 202));
        addAndMakeVisible (seq.division);
        seq.divA = std::make_unique<ComboAttachment> (processor.getAPVTS(), epId (v, "division"), seq.division);

        styleActionButton (seq.evolve, accent, false);
        styleActionButton (seq.drift, accent, true);
        styleActionButton (seq.autoMode, accent, true);
        seq.evolve.onClick = [this, v] { processor.requestEvolve (v); };
        seq.driftA = std::make_unique<ButtonAttachment> (processor.getAPVTS(), epId (v, "drift"), seq.drift);
        seq.autoA = std::make_unique<ButtonAttachment> (processor.getAPVTS(), epId (v, "auto"), seq.autoMode);

        styleSmallKnob (seq.density, seq.densityL, "DENS", accent);
        styleHorizontal (seq.prob, seq.probL, "PROB", accent);
        styleSmallKnob (seq.rotate, seq.rotateL, "ROT", accent);
        seq.densityA = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "density"), seq.density);
        seq.probA = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "prob"), seq.prob);
        seq.rotateA = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "rotate"), seq.rotate);

        seq.pattern = std::make_unique<PatternView> (processor, v, accent);
        addAndMakeVisible (*seq.pattern);

        auto& e = engines[(size_t) v];
        e.title.setText ("VOICE " + juce::String (v + 1), juce::dontSendNotification);
        e.title.setJustificationType (juce::Justification::centred);
        e.title.setColour (juce::Label::textColourId, accent.darker (0.32f));
        e.title.setFont (juce::Font (juce::FontOptions (14.5f)).boldened());
        addAndMakeVisible (e.title);

        styleActionButton (e.mute, accent, true);
        e.muteA = std::make_unique<ButtonAttachment> (processor.getAPVTS(), epId (v, "mute"), e.mute);

        const std::array<const char*, 4> engineNames { "PITCH", "TONE", "FM", "NOISE" };
        const std::array<const char*, 4> engineIds { "pitch", "tone", "fm", "noise" };
        for (int k = 0; k < 4; ++k)
        {
            styleKnob (e.knobs[(size_t) k], e.labels[(size_t) k], engineNames[(size_t) k], accent, false);
            e.attachments[(size_t) k] = std::make_unique<SliderAttachment> (
                processor.getAPVTS(), epId (v, engineIds[(size_t) k]), e.knobs[(size_t) k]);
        }

        styleCombo (e.wave, e.waveL, "WAVE");
        e.wave.addItemList (waveNames, 1);
        e.waveA = std::make_unique<ComboAttachment> (processor.getAPVTS(), epId (v, "wave"), e.wave);

        styleHorizontal (e.level, e.levelL, "LEVEL", accent);
        e.levelA = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "level"), e.level);

        auto& m = macros[(size_t) v];
        m.title.setText ("EVERYTHING " + juce::String (v + 1), juce::dontSendNotification);
        m.title.setJustificationType (juce::Justification::centredLeft);
        m.title.setColour (juce::Label::textColourId, juce::Colour::fromRGB (27, 29, 31));
        m.title.setFont (juce::Font (juce::FontOptions (13.5f)).boldened());
        addAndMakeVisible (m.title);

        styleKnob (m.amount, m.amountL, "AMOUNT", accent, true);
        styleCombo (m.shape, m.shapeL, "SHAPE");
        styleCombo (m.rate, m.rateL, "RATE");
        m.shape.addItemList (lfoShapes, 1);
        m.rate.addItemList (lfoRates, 1);

        m.amountA = std::make_unique<SliderAttachment> (processor.getAPVTS(), epId (v, "everything"), m.amount);
        m.shapeA = std::make_unique<ComboAttachment> (processor.getAPVTS(), epId (v, "everything_shape"), m.shape);
        m.rateA = std::make_unique<ComboAttachment> (processor.getAPVTS(), epId (v, "everything_rate"), m.rate);
    }

    const std::array<const char*, 10> globalNames {
        "DECAY", "ENV", "BASE", "BLAST", "RES", "DRIVE", "RANDOM", "VAR", "INTERACT", "CHAOS"
    };
    const std::array<const char*, 10> globalIds {
        "decay", "env_amt", "filter_base", "filter_blast", "res", "drive", "random", "variation", "interact", "chaos"
    };

    for (int i = 0; i < 10; ++i)
    {
        styleKnob (globals[(size_t) i], globalLabels[(size_t) i], globalNames[(size_t) i],
                   juce::Colour::fromRGB (42, 45, 49), false);
        globalAttachments[(size_t) i] = std::make_unique<SliderAttachment> (
            processor.getAPVTS(), globalIds[(size_t) i], globals[(size_t) i]);
    }

    const std::array<const char*, 4> crusherNames { "CRUSH", "BITS", "MIX", "LFO" };
    const std::array<const char*, 4> crusherIds { "crush", "bits", "crush_mix", "crush_lfo_depth" };
    for (int i = 0; i < 4; ++i)
    {
        styleKnob (crusherKnobs[(size_t) i], crusherLabels[(size_t) i], crusherNames[(size_t) i],
                   i == 3 ? voiceColour (0) : juce::Colour::fromRGB (45, 47, 50), false);
        crusherAttachments[(size_t) i] = std::make_unique<SliderAttachment> (
            processor.getAPVTS(), crusherIds[(size_t) i], crusherKnobs[(size_t) i]);
    }

    styleCombo (crusherShape, crusherShapeL, "SHAPE");
    styleCombo (crusherRate, crusherRateL, "RATE (SYNC)");
    crusherShape.addItemList (lfoShapes, 1);
    crusherRate.addItemList (lfoRates, 1);
    crusherShapeA = std::make_unique<ComboAttachment> (processor.getAPVTS(), "crush_lfo_shape", crusherShape);
    crusherRateA = std::make_unique<ComboAttachment> (processor.getAPVTS(), "crush_lfo_rate", crusherRate);

    styleHorizontal (output, outputL, "OUTPUT", juce::Colour::fromRGB (55, 58, 61));
    styleHorizontal (width, widthL, "WIDTH", juce::Colour::fromRGB (55, 58, 61));
    outputA = std::make_unique<SliderAttachment> (processor.getAPVTS(), "output", output);
    widthA = std::make_unique<SliderAttachment> (processor.getAPVTS(), "width", width);

    styleActionButton (mutate, voiceColour (0), false);
    mutate.onClick = [this] { processor.mutate(); };
}

PlasmaPercAudioProcessorEditor::~PlasmaPercAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

juce::Colour PlasmaPercAudioProcessorEditor::voiceColour (int index) const
{
    static const std::array<juce::Colour, 4> colours {
        juce::Colour::fromRGB (143, 112, 205),
        juce::Colour::fromRGB (205, 103, 132),
        juce::Colour::fromRGB (89, 157, 201),
        juce::Colour::fromRGB (125, 127, 126)
    };
    return colours[(size_t) juce::jlimit (0, 3, index)];
}

void PlasmaPercAudioProcessorEditor::styleKnob (
    juce::Slider& s, juce::Label& l, const juce::String& name, juce::Colour accent, bool large)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setColour (juce::Slider::rotarySliderFillColourId, accent);
    s.setMouseDragSensitivity (large ? 240 : 180);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, juce::Colour::fromRGB (28, 30, 32));
    l.setFont (juce::Font (juce::FontOptions (12.5f)).boldened());
    addAndMakeVisible (l);
}

void PlasmaPercAudioProcessorEditor::styleSmallKnob (
    juce::Slider& s, juce::Label& l, const juce::String& name, juce::Colour accent)
{
    styleKnob (s, l, name, accent, false);
    s.setMouseDragSensitivity (130);
}

void PlasmaPercAudioProcessorEditor::styleHorizontal (
    juce::Slider& s, juce::Label& l, const juce::String& name, juce::Colour accent)
{
    s.setSliderStyle (juce::Slider::LinearHorizontal);
    s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    s.setColour (juce::Slider::trackColourId, accent);
    s.setColour (juce::Slider::backgroundColourId, juce::Colour::fromRGB (218, 220, 219));
    s.setColour (juce::Slider::thumbColourId, accent.darker (0.15f));
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centredLeft);
    l.setColour (juce::Label::textColourId, juce::Colour::fromRGB (28, 30, 32));
    l.setFont (juce::Font (juce::FontOptions (12.0f)).boldened());
    addAndMakeVisible (l);
}

void PlasmaPercAudioProcessorEditor::styleCombo (
    juce::ComboBox& box, juce::Label& l, const juce::String& name)
{
    box.setColour (juce::ComboBox::backgroundColourId, juce::Colour::fromRGB (248, 248, 246));
    box.setColour (juce::ComboBox::textColourId, juce::Colour::fromRGB (24, 26, 28));
    box.setColour (juce::ComboBox::outlineColourId, juce::Colour::fromRGB (198, 200, 200));
    addAndMakeVisible (box);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centredLeft);
    l.setColour (juce::Label::textColourId, juce::Colour::fromRGB (31, 33, 35));
    l.setFont (juce::Font (juce::FontOptions (11.5f)).boldened());
    addAndMakeVisible (l);
}

void PlasmaPercAudioProcessorEditor::styleActionButton (
    juce::TextButton& b, juce::Colour accent, bool toggle)
{
    b.setClickingTogglesState (toggle);
    b.setColour (juce::TextButton::buttonColourId, juce::Colour::fromRGB (246, 246, 244));
    b.setColour (juce::TextButton::buttonOnColourId, accent.withAlpha (0.26f));
    b.setColour (juce::TextButton::textColourOffId, juce::Colour::fromRGB (28, 30, 32));
    b.setColour (juce::TextButton::textColourOnId, juce::Colour::fromRGB (20, 22, 24));
    addAndMakeVisible (b);
}

void PlasmaPercAudioProcessorEditor::drawPanel (
    juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const
{
    g.setColour (juce::Colour::fromRGB (249, 249, 247));
    g.fillRoundedRectangle (r.toFloat(), 9.0f);
    g.setColour (juce::Colour::fromRGB (209, 211, 211));
    g.drawRoundedRectangle (r.toFloat(), 9.0f, 1.0f);

    g.setColour (juce::Colour::fromRGB (27, 29, 31));
    g.setFont (juce::Font (juce::FontOptions (16.0f)).boldened());
    g.drawText (title, r.reduced (14, 7).removeFromTop (24), juce::Justification::centredLeft);
}

void PlasmaPercAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour::fromRGB (241, 242, 240));

    const int headerH = juce::roundToInt ((float) getHeight() * 0.115f);
    auto body = getLocalBounds().withTrimmedTop (headerH).withTrimmedBottom (56).reduced (10, 0);

    const int seqW = juce::roundToInt ((float) body.getWidth() * 0.31f);
    const int engW = juce::roundToInt ((float) body.getWidth() * 0.27f);
    const int globalW = juce::roundToInt ((float) body.getWidth() * 0.25f);

    auto seqArea = body.removeFromLeft (seqW).reduced (4);
    auto engineArea = body.removeFromLeft (engW).reduced (4);
    auto globalArea = body.removeFromLeft (globalW).reduced (4);
    auto macroArea = body.reduced (4);

    drawPanel (g, seqArea, "SEQUENCERS");
    drawPanel (g, engineArea, "VOICE ENGINES");
    drawPanel (g, globalArea, "ENV / FILTER / GENERATIVE");
    drawPanel (g, macroArea, "EVERYTHING MACROS");

    g.setColour (juce::Colour::fromRGB (20, 22, 24));
    g.setFont (juce::Font (juce::FontOptions (29.0f)).boldened());
    g.drawText ("P L A S M A T I K", 0, 28, getWidth(), 34, juce::Justification::centred);

    if (fairyImage.isValid())
    {
        g.setOpacity (0.88f);
        const int logoH = juce::jmax (45, headerH - 10);
        g.drawImageWithin (fairyImage, getWidth() / 2 - 35, 1, 70, logoH,
                           juce::RectanglePlacement::centred, false);

        g.setOpacity (0.12f);
        auto art = macroArea.reduced (12).withTrimmedLeft (macroArea.getWidth() / 3);
        g.drawImageWithin (fairyImage, art.getX(), art.getY(), art.getWidth(), art.getHeight(),
                           juce::RectanglePlacement::centred, false);
        g.setOpacity (1.0f);
    }
}

void PlasmaPercAudioProcessorEditor::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int headerH = juce::roundToInt ((float) h * 0.115f);
    const int bottomH = 56;

    preset.setBounds (20, juce::jmax (14, headerH / 2 - 18), juce::jmin (300, w / 4), 36);
    subtitle.setBounds (w / 2 - juce::jmin (270, w / 5), headerH - 28,
                        juce::jmin (540, w * 2 / 5), 18);

    auto body = getLocalBounds().withTrimmedTop (headerH).withTrimmedBottom (bottomH).reduced (10, 0);

    const int seqW = juce::roundToInt ((float) body.getWidth() * 0.31f);
    const int engW = juce::roundToInt ((float) body.getWidth() * 0.27f);
    const int globalW = juce::roundToInt ((float) body.getWidth() * 0.25f);

    auto seqArea = body.removeFromLeft (seqW).reduced (14, 10);
    auto engineArea = body.removeFromLeft (engW).reduced (14, 10);
    auto globalArea = body.removeFromLeft (globalW).reduced (14, 10);
    auto macroArea = body.reduced (14, 10);

    // Sequencers --------------------------------------------------------------
    seqArea.removeFromTop (34);
    const int seqUtilityH = 42;
    auto seqRowsArea = seqArea.withTrimmedBottom (seqUtilityH);
    const int rowH = seqRowsArea.getHeight() / 4;

    for (int v = 0; v < 4; ++v)
    {
        auto row = seqRowsArea.removeFromTop (rowH).reduced (0, 3);
        auto top = row.removeFromTop (30);

        seqs[(size_t) v].title.setBounds (top.removeFromLeft (70));
        seqs[(size_t) v].division.setBounds (top.removeFromLeft (72).reduced (2, 1));
        seqs[(size_t) v].evolve.setBounds (top.removeFromLeft (72).reduced (2, 1));
        seqs[(size_t) v].drift.setBounds (top.removeFromLeft (62).reduced (2, 1));
        seqs[(size_t) v].autoMode.setBounds (top.removeFromLeft (62).reduced (2, 1));

        auto graph = row.removeFromTop (juce::jmax (38, row.getHeight() / 2));
        seqs[(size_t) v].pattern->setBounds (graph.reduced (2));

        auto controls = row.reduced (2, 0);
        const int third = controls.getWidth() / 3;

        auto dens = controls.removeFromLeft (third);
        seqs[(size_t) v].densityL.setBounds (dens.removeFromLeft (46));
        seqs[(size_t) v].density.setBounds (dens.withSizeKeepingCentre (44, juce::jmin (44, dens.getHeight())));

        auto prob = controls.removeFromLeft (third);
        seqs[(size_t) v].probL.setBounds (prob.removeFromLeft (44));
        seqs[(size_t) v].prob.setBounds (prob.reduced (2, 2));

        auto rot = controls;
        seqs[(size_t) v].rotateL.setBounds (rot.removeFromLeft (35));
        seqs[(size_t) v].rotate.setBounds (rot.withSizeKeepingCentre (44, juce::jmin (44, rot.getHeight())));
    }

    // Voice engines -----------------------------------------------------------
    engineArea.removeFromTop (34);
    const int engineW = engineArea.getWidth() / 4;

    for (int v = 0; v < 4; ++v)
    {
        auto col = engineArea.removeFromLeft (engineW).reduced (3, 0);
        auto& e = engines[(size_t) v];

        e.title.setBounds (col.removeFromTop (23));
        e.mute.setBounds (col.removeFromTop (28).reduced (8, 2));

        for (int k = 0; k < 4; ++k)
        {
            auto cell = col.removeFromTop (juce::jmax (60, col.getHeight() / (6 - k)));
            e.labels[(size_t) k].setBounds (cell.removeFromTop (17));
            e.knobs[(size_t) k].setBounds (cell.withSizeKeepingCentre (
                juce::jmin (72, cell.getWidth()), juce::jmin (64, cell.getHeight())));
        }

        auto waveCell = col.removeFromTop (46);
        e.waveL.setBounds (waveCell.removeFromTop (15));
        e.wave.setBounds (waveCell.reduced (4, 2));

        auto levelCell = col;
        e.levelL.setBounds (levelCell.removeFromTop (16));
        e.level.setBounds (levelCell.reduced (5, 2));
    }

    // Global / filter / generative + crusher ---------------------------------
    globalArea.removeFromTop (34);
    const int crusherH = juce::jmax (175, globalArea.getHeight() / 3);
    auto globalsArea = globalArea.withTrimmedBottom (crusherH + 8);
    auto crusherArea = globalArea.removeFromBottom (crusherH);

    const int gw = globalsArea.getWidth() / 3;
    const int gh = globalsArea.getHeight() / 4;

    for (int i = 0; i < 9; ++i)
    {
        const int col = i % 3;
        const int row = i / 3;
        auto cell = juce::Rectangle<int> (
            globalsArea.getX() + col * gw,
            globalsArea.getY() + row * gh,
            gw, gh).reduced (4, 2);

        globalLabels[(size_t) i].setBounds (cell.removeFromBottom (18));
        globals[(size_t) i].setBounds (cell.withSizeKeepingCentre (
            juce::jmin (72, cell.getWidth()), juce::jmin (68, cell.getHeight())));
    }

    auto bottomGlobal = juce::Rectangle<int> (
        globalsArea.getX(), globalsArea.getY() + 3 * gh, globalsArea.getWidth(), gh).reduced (4, 2);
    auto chaosCell = bottomGlobal.removeFromLeft (gw);
    globalLabels[9].setBounds (chaosCell.removeFromBottom (18));
    globals[9].setBounds (chaosCell.withSizeKeepingCentre (
        juce::jmin (72, chaosCell.getWidth()), juce::jmin (68, chaosCell.getHeight())));
    mutate.setBounds (bottomGlobal.reduced (16, juce::jmax (6, bottomGlobal.getHeight() / 4)));

    const int crusherTitleH = 24;
    auto crusherTop = crusherArea.removeFromTop (crusherTitleH);
    crusherTop.removeFromLeft (4);
    auto crusherLabel = crusherTop;
    (void) crusherLabel;

    auto knobRow = crusherArea.removeFromTop (juce::jmax (78, crusherArea.getHeight() / 2));
    const int kw = knobRow.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
    {
        auto cell = knobRow.removeFromLeft (kw).reduced (2);
        crusherLabels[(size_t) i].setBounds (cell.removeFromBottom (17));
        crusherKnobs[(size_t) i].setBounds (cell.withSizeKeepingCentre (
            juce::jmin (62, cell.getWidth()), juce::jmin (58, cell.getHeight())));
    }

    auto comboRow = crusherArea;
    auto shapeCell = comboRow.removeFromLeft (comboRow.getWidth() / 2).reduced (3);
    crusherShapeL.setBounds (shapeCell.removeFromTop (16));
    crusherShape.setBounds (shapeCell.removeFromTop (30));
    auto rateCell = comboRow.reduced (3);
    crusherRateL.setBounds (rateCell.removeFromTop (16));
    crusherRate.setBounds (rateCell.removeFromTop (30));

    // Everything LFO modules --------------------------------------------------
    macroArea.removeFromTop (34);
    const int macroH = macroArea.getHeight() / 4;
    for (int v = 0; v < 4; ++v)
    {
        auto row = macroArea.removeFromTop (macroH).reduced (2, 4);
        auto& m = macros[(size_t) v];

        m.title.setBounds (row.removeFromTop (20));

        const int amountW = juce::jmax (72, row.getWidth() / 2);
        auto amountCell = row.removeFromLeft (amountW);
        m.amountL.setBounds (amountCell.removeFromBottom (17));
        m.amount.setBounds (amountCell.withSizeKeepingCentre (
            juce::jmin (84, amountCell.getWidth()), juce::jmin (74, amountCell.getHeight())));

        auto selects = row.reduced (2);
        auto sh = selects.removeFromTop (selects.getHeight() / 2);
        m.shapeL.setBounds (sh.removeFromTop (15));
        m.shape.setBounds (sh.reduced (1, 1));
        auto rt = selects;
        m.rateL.setBounds (rt.removeFromTop (15));
        m.rate.setBounds (rt.reduced (1, 1));
    }

    // Bottom utilities --------------------------------------------------------
    auto bottom = getLocalBounds().removeFromBottom (bottomH).reduced (18, 7);
    auto leftBottom = bottom.removeFromLeft (bottom.getWidth() / 2);
    outputL.setBounds (leftBottom.removeFromLeft (64));
    output.setBounds (leftBottom.reduced (4, 3));

    auto rightBottom = bottom;
    widthL.setBounds (rightBottom.removeFromLeft (54));
    width.setBounds (rightBottom.reduced (4, 3));
}

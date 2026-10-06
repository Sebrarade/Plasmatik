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

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::drawLinearSlider (
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float, float, const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, 0.0f, 1.0f, style, slider);
        return;
    }

    auto r = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (3.0f, height * 0.34f);
    const float radius = r.getHeight() * 0.5f;

    g.setColour (juce::Colour::fromRGB (220, 221, 219));
    g.fillRoundedRectangle (r, radius);

    auto filled = r;
    filled.setWidth (juce::jlimit (0.0f, r.getWidth(), sliderPos - r.getX()));
    g.setColour (slider.findColour (juce::Slider::trackColourId));
    g.fillRoundedRectangle (filled, radius);

    const float thumbX = juce::jlimit (r.getX(), r.getRight(), sliderPos);
    g.setColour (juce::Colour::fromRGB (245, 245, 242));
    g.fillEllipse (thumbX - 5.0f, r.getCentreY() - 5.0f, 10.0f, 10.0f);
    g.setColour (juce::Colour::fromRGB (166, 168, 168));
    g.drawEllipse (thumbX - 5.0f, r.getCentreY() - 5.0f, 10.0f, 10.0f, 1.0f);
}

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::drawComboBox (
    juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0.5f, 0.5f, (float) width - 1.0f, (float) height - 1.0f);
    g.setColour (juce::Colour::fromRGB (249, 249, 247));
    g.fillRoundedRectangle (r, 4.5f);
    g.setColour (juce::Colour::fromRGB (197, 199, 199));
    g.drawRoundedRectangle (r, 4.5f, 1.0f);

    const float cx = (float) width - 15.0f;
    const float cy = (float) height * 0.50f;
    juce::Path p;
    p.startNewSubPath (cx - 4.0f, cy - 2.0f);
    p.lineTo (cx, cy + 2.0f);
    p.lineTo (cx + 4.0f, cy - 2.0f);
    g.setColour (box.findColour (juce::ComboBox::textColourId));
    g.strokePath (p, juce::PathStrokeType (1.7f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::positionComboBoxText (
    juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (9, 1, box.getWidth() - 29, box.getHeight() - 2);
    label.setFont (juce::Font (juce::FontOptions (13.0f)).boldened());
    label.setJustificationType (juce::Justification::centredLeft);
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
    setResizeLimits (1115, 627, 2006, 1129);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio (1672.0 / 941.0);
    setSize (1672, 941);

    fairyImage = juce::ImageCache::getFromMemory (BinaryData::fairy_logo_jpg, BinaryData::fairy_logo_jpgSize);


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
        styleSmallKnob (seq.prob, seq.probL, "PROB", accent);
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

        styleCombo (e.wave, e.waveL, "");
        e.wave.addItemList (waveNames, 1);
        e.waveA = std::make_unique<ComboAttachment> (processor.getAPVTS(), epId (v, "wave"), e.wave);

        styleKnob (e.level, e.levelL, "LEVEL", accent, false);
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
    const float sx = (float) getWidth() / 1672.0f;
    const float sy = (float) getHeight() / 941.0f;
    auto R = [sx, sy] (float x, float y, float w, float h)
    {
        return juce::Rectangle<float> (x * sx, y * sy, w * sx, h * sy);
    };

    // Approved light/off-white shell.
    g.fillAll (juce::Colour::fromRGB (242, 243, 241));

    // Very subtle top haze like the approved mockup.
    juce::ColourGradient topGlow (juce::Colour::fromRGB (252, 252, 250), 836.0f * sx, 0.0f,
                                  juce::Colour::fromRGB (238, 240, 239), 836.0f * sx, 150.0f * sy, false);
    g.setGradientFill (topGlow);
    g.fillRect (R (0, 0, 1672, 150));

    // Main cards – same proportions as the approved artwork.
    drawPanel (g, R (10, 143, 502, 774).toNearestInt(), "SEQUENCERS");
    drawPanel (g, R (520, 143, 390, 774).toNearestInt(), "VOICE ENGINES");
    drawPanel (g, R (918, 143, 372, 511).toNearestInt(), "ENV / FILTER / GENERATIVE");
    drawPanel (g, R (918, 663, 372, 254).toNearestInt(), "BITCRUSHER (REDUX)");
    drawPanel (g, R (1298, 143, 364, 774).toNearestInt(), "EVERYTHING MACROS");

    // Thin separators in the four track / macro lanes.
    g.setColour (juce::Colour::fromRGB (224, 225, 223));
    for (int i = 1; i < 4; ++i)
    {
        const float y = (201.0f + i * 160.0f) * sy;
        g.drawLine (28.0f * sx, y, 497.0f * sx, y, 1.0f);
    }
    for (int i = 1; i < 4; ++i)
    {
        const float y = (201.0f + i * 174.0f) * sy;
        g.drawLine (1312.0f * sx, y, 1648.0f * sx, y, 1.0f);
    }

    // Header utility marks.
    g.setColour (juce::Colour::fromRGB (28, 30, 32));
    g.setFont (juce::Font (juce::FontOptions (30.0f * sx)).boldened());
    g.drawText ("◯", R (28, 72, 42, 42).toNearestInt(), juce::Justification::centred);
    g.setFont (juce::Font (juce::FontOptions (25.0f * sx)));
    g.drawText ("‹", R (100, 72, 28, 42).toNearestInt(), juce::Justification::centred);
    g.drawText ("›", R (135, 72, 28, 42).toNearestInt(), juce::Justification::centred);

    // Fairy branding: compact mark above the title + large pale right-hand watermark.
    if (fairyImage.isValid())
    {
        g.setOpacity (0.98f);
        auto logo = R (642, 1, 100, 116).toNearestInt();
        g.drawImageWithin (fairyImage, logo.getX(), logo.getY(), logo.getWidth(), logo.getHeight(),
                           juce::RectanglePlacement::centred, false);

        g.setOpacity (0.18f);
        auto art = R (1452, 180, 205, 650).toNearestInt();
        g.drawImageWithin (fairyImage, art.getX(), art.getY(), art.getWidth(), art.getHeight(),
                           juce::RectanglePlacement::centred, false);
        g.setOpacity (1.0f);
    }

    // PLASMATIK only — subtitle intentionally removed.
    g.setColour (juce::Colour::fromRGB (18, 20, 23));
    g.setFont (juce::Font (juce::FontOptions (31.0f * sx)));
    g.drawText ("P  L  A  S  M  A  T  I  K", R (715, 69, 430, 43).toNearestInt(), juce::Justification::centred);

    // Top-right utility labels.
    g.setFont (juce::Font (juce::FontOptions (13.5f * sx)).boldened());
    g.drawText ("◇", R (1362, 73, 44, 42).toNearestInt(), juce::Justification::centred);
    g.drawText ("INIT", R (1422, 78, 52, 32).toNearestInt(), juce::Justification::centred);
    g.drawText ("SAVE", R (1494, 78, 58, 32).toNearestInt(), juce::Justification::centred);
    g.setFont (juce::Font (juce::FontOptions (24.0f * sx)).boldened());
    g.drawText ("•••", R (1576, 76, 56, 32).toNearestInt(), juce::Justification::centred);

    // Small visual utility row under the sequencer – matches the approved design.
    auto util = R (28, 826, 468, 47);
    g.setColour (juce::Colour::fromRGB (246, 247, 245));
    g.fillRoundedRectangle (util, 5.0f * sx);
    g.setColour (juce::Colour::fromRGB (204, 206, 205));
    g.drawRoundedRectangle (util, 5.0f * sx, 1.0f);

    auto drawMini = [&] (juce::String text, juce::Rectangle<float> r)
    {
        g.setColour (juce::Colour::fromRGB (249, 249, 247));
        g.fillRoundedRectangle (r, 4.0f * sx);
        g.setColour (juce::Colour::fromRGB (198, 200, 200));
        g.drawRoundedRectangle (r, 4.0f * sx, 1.0f);
        g.setColour (juce::Colour::fromRGB (29, 31, 33));
        g.setFont (juce::Font (juce::FontOptions (12.0f * sx)).boldened());
        g.drawText (text, r.toNearestInt(), juce::Justification::centred);
    };

    drawMini ("▶", R (31, 831, 50, 35));
    g.setColour (juce::Colour::fromRGB (29, 31, 33));
    g.setFont (juce::Font (juce::FontOptions (12.0f * sx)).boldened());
    g.drawText ("STEPS", R (93, 831, 54, 35).toNearestInt(), juce::Justification::centredLeft);
    drawMini ("16⌄", R (145, 831, 72, 35));
    drawMini ("COPY", R (242, 831, 59, 35));
    drawMini ("PASTE", R (306, 831, 64, 35));
    drawMini ("CLEAR", R (374, 831, 65, 35));
    drawMini ("⤨", R (444, 831, 46, 35));

    // Crusher heading accent + power glyph.
    g.setColour (juce::Colour::fromRGB (25, 27, 29));
    g.setFont (juce::Font (juce::FontOptions (13.0f * sx)).boldened());
    g.drawText ("⌁", R (930, 669, 30, 28).toNearestInt(), juce::Justification::centred);
    g.drawText ("POWER", R (1220, 669, 58, 28).toNearestInt(), juce::Justification::centredRight);

    // Bottom master labels.
    g.setColour (juce::Colour::fromRGB (28, 30, 32));
    g.setFont (juce::Font (juce::FontOptions (12.0f * sx)).boldened());
    g.drawText ("OUTPUT", R (32, 882, 68, 28).toNearestInt(), juce::Justification::centredLeft);
    g.drawText ("MIX", R (1150, 882, 50, 28).toNearestInt(), juce::Justification::centredRight);
    g.drawText ("WIDTH", R (1392, 882, 62, 28).toNearestInt(), juce::Justification::centredRight);
}

void PlasmaPercAudioProcessorEditor::resized()
{
    const float sx = (float) getWidth() / 1672.0f;
    const float sy = (float) getHeight() / 941.0f;
    auto B = [sx, sy] (int x, int y, int w, int h)
    {
        return juce::Rectangle<int> (juce::roundToInt ((float) x * sx),
                                     juce::roundToInt ((float) y * sy),
                                     juce::roundToInt ((float) w * sx),
                                     juce::roundToInt ((float) h * sy));
    };

    // Header preset field.
    preset.setBounds (B (179, 78, 326, 45));

    // Sequencer tracks.
    const std::array<int, 4> seqY { 208, 369, 531, 692 };
    for (int v = 0; v < 4; ++v)
    {
        auto& s = seqs[(size_t) v];
        const int y = seqY[(size_t) v];

        s.title.setBounds (B (38, y, 92, 31));
        s.division.setBounds (B (132, y, 83, 35));
        s.evolve.setBounds (B (229, y, 77, 35));
        s.drift.setBounds (B (316, y, 68, 35));
        s.autoMode.setBounds (B (394, y, 67, 35));

        if (s.pattern != nullptr)
            s.pattern->setBounds (B (38, y + 48, 432, 63));

        s.densityL.setBounds (B (39, y + 111, 48, 28));
        s.density.setBounds (B (80, y + 104, 42, 42));

        s.probL.setBounds (B (233, y + 111, 48, 28));
        s.prob.setBounds (B (273, y + 104, 42, 42));

        s.rotateL.setBounds (B (389, y + 111, 44, 28));
        s.rotate.setBounds (B (421, y + 104, 42, 42));
    }

    // Voice engines — four columns matching the approved mockup.
    const std::array<int, 4> vx { 526, 622, 718, 814 };
    for (int v = 0; v < 4; ++v)
    {
        auto& e = engines[(size_t) v];
        const int x = vx[(size_t) v];

        e.title.setBounds (B (x, 207, 82, 27));
        e.mute.setBounds (B (x + 2, 244, 78, 32));

        const std::array<int, 4> ky { 292, 405, 518, 631 };
        for (int k = 0; k < 4; ++k)
        {
            e.knobs[(size_t) k].setBounds (B (x + 7, ky[(size_t) k], 68, 68));
            e.labels[(size_t) k].setBounds (B (x, ky[(size_t) k] + 69, 82, 24));
        }

        e.waveL.setVisible (false);
        e.wave.setBounds (B (x + 2, 725, 78, 38));
        e.levelL.setBounds (B (x, 771, 82, 23));
        e.level.setBounds (B (x + 8, 794, 66, 66));
    }

    // ENV / FILTER / GENERATIVE – 3 x 3 grid + chaos + mutate.
    const std::array<int, 3> gx { 942, 1061, 1180 };
    const std::array<int, 3> gy { 207, 322, 437 };
    int gi = 0;
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            globals[(size_t) gi].setBounds (B (gx[(size_t) col], gy[(size_t) row], 72, 72));
            globalLabels[(size_t) gi].setBounds (B (gx[(size_t) col] - 5, gy[(size_t) row] + 74, 82, 24));
            ++gi;
        }
    }

    globals[9].setBounds (B (942, 550, 72, 72));
    globalLabels[9].setBounds (B (937, 624, 82, 24));
    mutate.setBounds (B (1043, 555, 218, 69));

    // Bitcrusher.
    const std::array<int, 4> cx { 937, 997, 1057, 1117 };
    for (int i = 0; i < 4; ++i)
    {
        crusherKnobs[(size_t) i].setBounds (B (cx[(size_t) i], 716, 58, 58));
        crusherLabels[(size_t) i].setBounds (B (cx[(size_t) i] - 2, 775, 62, 22));
    }

    crusherShapeL.setBounds (B (1130, 704, 95, 20));
    crusherShape.setBounds (B (1130, 726, 142, 36));
    crusherRateL.setBounds (B (1130, 769, 110, 20));
    crusherRate.setBounds (B (1130, 792, 142, 38));

    // Everything macros.
    const std::array<int, 4> my { 207, 381, 555, 729 };
    for (int v = 0; v < 4; ++v)
    {
        auto& m = macros[(size_t) v];
        const int y = my[(size_t) v];

        m.title.setBounds (B (1324, y, 150, 27));
        m.amount.setBounds (B (1318, y + 35, 86, 86));
        m.amountL.setBounds (B (1318, y + 121, 86, 24));

        m.shapeL.setBounds (B (1428, y + 31, 78, 20));
        m.shape.setBounds (B (1428, y + 52, 112, 38));
        m.rateL.setBounds (B (1428, y + 96, 78, 20));
        m.rate.setBounds (B (1428, y + 117, 112, 38));
    }

    // Bottom master strips.
    outputL.setVisible (false);
    output.setBounds (B (111, 883, 198, 27));
    widthL.setVisible (false);
    width.setBounds (B (1453, 883, 168, 27));
}

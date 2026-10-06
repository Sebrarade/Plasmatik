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
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height);
    const float d = juce::jmin (bounds.getWidth(), bounds.getHeight()) - 10.0f;
    const float radius = d * 0.5f;
    const auto centre = bounds.getCentre();
    const auto knob = juce::Rectangle<float> (centre.x - radius, centre.y - radius, d, d);

    g.setColour (juce::Colour::fromRGBA (25, 27, 29, 26));
    g.fillEllipse (knob.translated (3.0f, 4.0f));

    juce::ColourGradient face (juce::Colour::fromRGB (250, 250, 248), knob.getX(), knob.getY(),
                               juce::Colour::fromRGB (212, 213, 211), knob.getRight(), knob.getBottom(), false);
    face.addColour (0.45, juce::Colour::fromRGB (239, 239, 237));
    g.setGradientFill (face);
    g.fillEllipse (knob);
    g.setColour (juce::Colour::fromRGB (184, 187, 186));
    g.drawEllipse (knob, 1.1f);

    const float arcRadius = radius + 4.0f;
    juce::Path darkArc;
    darkArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                           rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour::fromRGB (22, 25, 27));
    g.strokePath (darkArc, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    const float valueEnd = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                            rotaryStartAngle, valueEnd, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
    g.strokePath (valueArc, juce::PathStrokeType (3.4f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    const auto dot = centre + juce::Point<float> (std::sin (valueEnd), -std::cos (valueEnd)) * (radius * 0.58f);
    g.setColour (juce::Colour::fromRGB (19, 21, 23));
    g.fillEllipse (dot.x - 2.0f, dot.y - 2.0f, 4.0f, 4.0f);
}

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::drawButtonBackground (
    juce::Graphics& g, juce::Button& button, const juce::Colour&, bool isHighlighted, bool isDown)
{
    auto r = button.getLocalBounds().toFloat().reduced (0.6f);
    const auto accent = button.findColour (juce::TextButton::buttonOnColourId);
    const bool on = button.getToggleState();

    auto fill = juce::Colour::fromRGB (250, 250, 248);
    if (on) fill = accent.withMultipliedAlpha (0.12f).overlaidWith (fill.withAlpha (0.88f));
    if (isHighlighted) fill = fill.darker (0.025f);
    if (isDown) fill = fill.darker (0.055f);

    g.setColour (juce::Colour::fromRGBA (20, 22, 24, 16));
    g.fillRoundedRectangle (r.translated (0.0f, 1.5f), 4.5f);
    g.setColour (fill);
    g.fillRoundedRectangle (r, 4.5f);
    g.setColour (juce::Colour::fromRGB (198, 200, 200));
    g.drawRoundedRectangle (r, 4.5f, 1.0f);

    if (button.getButtonText() == "MUTE")
    {
        auto dot = juce::Rectangle<float> (8.0f, r.getCentreY() - 5.5f, 11.0f, 11.0f);
        g.setColour (accent.withAlpha (0.95f));
        g.drawEllipse (dot, 2.4f);
        if (on)
        {
            g.setColour (accent.withAlpha (0.28f));
            g.fillEllipse (dot.reduced (2.2f));
        }
    }

    if (button.getButtonText() == "MUTATE")
    {
        auto dice = juce::Rectangle<float> (17.0f, r.getCentreY() - 12.0f, 24.0f, 24.0f);
        g.setColour (juce::Colour::fromRGB (25, 27, 29));
        g.drawRoundedRectangle (dice, 3.0f, 1.6f);
        const std::array<juce::Point<float>,5> pts {{
            {dice.getX()+6.0f,dice.getY()+6.0f},{dice.getRight()-6.0f,dice.getY()+6.0f},
            {dice.getCentreX(),dice.getCentreY()},{dice.getX()+6.0f,dice.getBottom()-6.0f},
            {dice.getRight()-6.0f,dice.getBottom()-6.0f}}};
        for (auto p : pts) g.fillEllipse (p.x-1.8f,p.y-1.8f,3.6f,3.6f);
    }
}

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::drawButtonText (
    juce::Graphics& g, juce::TextButton& button, bool, bool)
{
    const auto colour = button.getToggleState()
        ? button.findColour (juce::TextButton::textColourOnId)
        : button.findColour (juce::TextButton::textColourOffId);

    auto r = button.getLocalBounds();
    if (button.getButtonText() == "MUTE")
        r.removeFromLeft (19);
    else if (button.getButtonText() == "MUTATE")
        r.removeFromLeft (35);

    g.setColour (colour);
    g.setFont (juce::Font (juce::FontOptions (button.getButtonText() == "MUTATE" ? 16.0f : 12.5f)).boldened());
    g.drawText (button.getButtonText(), r, juce::Justification::centred, true);
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
    auto track = juce::Rectangle<float> ((float)x+3.0f, (float)y+height*0.40f,
                                         (float)width-6.0f, juce::jmax (7.0f, height*0.20f));
    const float rad = track.getHeight()*0.5f;
    g.setColour (juce::Colour::fromRGB (221,222,220));
    g.fillRoundedRectangle (track,rad);
    auto fill=track;
    fill.setWidth (juce::jlimit (0.0f,track.getWidth(),sliderPos-track.getX()));
    g.setColour (slider.findColour (juce::Slider::trackColourId).withAlpha (0.92f));
    g.fillRoundedRectangle (fill,rad);
    const float tx=juce::jlimit(track.getX(),track.getRight(),sliderPos);
    g.setColour (juce::Colour::fromRGBA(20,22,24,25));
    g.fillEllipse(tx-6.0f,track.getCentreY()-4.5f,12.0f,12.0f);
    g.setColour(juce::Colour::fromRGB(246,246,244));
    g.fillEllipse(tx-6.0f,track.getCentreY()-6.0f,12.0f,12.0f);
    g.setColour(juce::Colour::fromRGB(169,171,170));
    g.drawEllipse(tx-6.0f,track.getCentreY()-6.0f,12.0f,12.0f,1.0f);
}

void PlasmaPercAudioProcessorEditor::PlasmatikLookAndFeel::drawComboBox (
    juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r=juce::Rectangle<float>(0.5f,0.5f,(float)width-1.0f,(float)height-1.0f);
    g.setColour(juce::Colour::fromRGBA(20,22,24,13));
    g.fillRoundedRectangle(r.translated(0.0f,1.2f),4.5f);
    g.setColour(juce::Colour::fromRGB(250,250,248));
    g.fillRoundedRectangle(r,4.5f);
    g.setColour(juce::Colour::fromRGB(200,202,201));
    g.drawRoundedRectangle(r,4.5f,1.0f);
    const float cx=(float)width-14.0f, cy=(float)height*0.50f;
    juce::Path p; p.startNewSubPath(cx-4.0f,cy-2.0f); p.lineTo(cx,cy+2.0f); p.lineTo(cx+4.0f,cy-2.0f);
    g.setColour(box.findColour(juce::ComboBox::textColourId));
    g.strokePath(p,juce::PathStrokeType(1.6f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
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
    auto area=getLocalBounds().toFloat().reduced(1.0f);
    const float stepW=area.getWidth()/16.0f;
    g.setColour(accent.withAlpha(0.26f));
    for(int s=0;s<16;++s){const float x=area.getX()+stepW*(float)s+stepW*0.5f;for(int d=0;d<5;++d)g.fillEllipse(x-1.0f,area.getBottom()-8.0f-d*8.0f,2.0f,2.0f);}
    juce::Path curve; bool started=false;
    for(int s=0;s<16;++s){
        const float p=processor.getPatternValue(voice,s);
        const float h=juce::jmap(p,0.0f,1.0f,5.0f,area.getHeight()*0.78f);
        const float x=area.getX()+stepW*(float)s+stepW*0.23f,w=juce::jmax(3.0f,stepW*0.34f),y=area.getBottom()-h;
        g.setColour(accent.withAlpha(0.72f)); g.fillRoundedRectangle(x,y,w,h,juce::jmin(1.5f,w*0.3f));
        auto pt=juce::Point<float>(x+w*0.5f,y-3.0f);
        if(!started){curve.startNewSubPath(pt);started=true;}else curve.lineTo(pt);
        g.setColour(accent.withAlpha(0.98f)); g.fillEllipse(pt.x-1.8f,pt.y-1.8f,3.6f,3.6f);
    }
    g.setColour(accent.withAlpha(0.83f));
    g.strokePath(curve,juce::PathStrokeType(1.55f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
}

//==============================================================================

PlasmaPercAudioProcessorEditor::PlasmaPercAudioProcessorEditor//==============================================================================

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
    if (fairyImage.isValid())
    {
        auto keyed = juce::Image (juce::Image::ARGB, fairyImage.getWidth(), fairyImage.getHeight(), true);
        { juce::Graphics fg (keyed); fg.drawImageAt (fairyImage, 0, 0); }
        juce::Image::BitmapData px (keyed, juce::Image::BitmapData::readWrite);
        for (int yy = 0; yy < keyed.getHeight(); ++yy)
            for (int xx = 0; xx < keyed.getWidth(); ++xx)
            {
                auto col = px.getPixelColour (xx, yy);
                const float bright = col.getPerceivedBrightness();
                const float sat = col.getSaturation();
                float alpha = 1.0f;
                if (bright > 0.90f && sat < 0.22f)
                    alpha = juce::jlimit (0.0f, 1.0f, (0.985f - bright) / 0.085f);
                px.setPixelColour (xx, yy, col.withAlpha (alpha));
            }
        fairyImage = keyed;
    }


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

    const juce::StringArray lfoShapes { "∿", "△", "╱", "□", "S&H", "≈RND" };
    const juce::StringArray lfoRates { "4 Bars", "2 Bars", "1 Bar", "1/2", "1/4", "1/8", "1/16", "1/32" };
    const juce::StringArray waveNames { "∿", "△", "▜", "╱", "✣" };

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
    b.setColour (juce::TextButton::buttonOnColourId, accent);
    b.setColour (juce::TextButton::textColourOffId, juce::Colour::fromRGB (28, 30, 32));
    b.setColour (juce::TextButton::textColourOnId, juce::Colour::fromRGB (20, 22, 24));
    addAndMakeVisible (b);
}

void PlasmaPercAudioProcessorEditor::drawPanel (
    juce::Graphics& g, juce::Rectangle<int> r, const juce::String& title) const
{
    auto rf=r.toFloat();
    g.setColour(juce::Colour::fromRGBA(20,22,24,14)); g.fillRoundedRectangle(rf.translated(0.0f,2.0f),7.0f);
    g.setColour(juce::Colour::fromRGB(249,249,247)); g.fillRoundedRectangle(rf,7.0f);
    g.setColour(juce::Colour::fromRGB(211,213,212)); g.drawRoundedRectangle(rf,7.0f,1.0f);

    auto header=r.reduced(15,7).removeFromTop(44);
    g.setColour(juce::Colour::fromRGB(25,27,29));
    g.setFont(juce::Font(juce::FontOptions(18.0f)).boldened());
    g.drawText(title,header.withTrimmedLeft(34),juce::Justification::centredLeft);

    auto ir=header.removeFromLeft(26).toFloat();
    juce::Path icon;
    if(title=="SEQUENCERS"){icon.startNewSubPath(ir.getX()+2,ir.getCentreY()+5);icon.cubicTo(ir.getX()+7,ir.getY()+3,ir.getX()+11,ir.getBottom()-3,ir.getX()+16,ir.getCentreY()-5);icon.cubicTo(ir.getX()+19,ir.getY()+5,ir.getX()+22,ir.getY()+8,ir.getRight()-1,ir.getY()+4);}
    else if(title=="VOICE ENGINES"){icon.startNewSubPath(ir.getX()+3,ir.getCentreY());icon.lineTo(ir.getX()+9,ir.getCentreY());icon.lineTo(ir.getX()+9,ir.getY()+4);icon.lineTo(ir.getX()+12,ir.getBottom()-4);icon.lineTo(ir.getX()+15,ir.getCentreY());icon.lineTo(ir.getRight()-2,ir.getCentreY());}
    else if(title.startsWith("ENV")){icon.addEllipse(ir.reduced(2));icon.addEllipse(ir.reduced(8));}
    else if(title.startsWith("BITCRUSHER")){icon.startNewSubPath(ir.getX()+2,ir.getBottom()-4);icon.lineTo(ir.getX()+9,ir.getBottom()-4);icon.lineTo(ir.getX()+9,ir.getCentreY());icon.lineTo(ir.getX()+16,ir.getCentreY());icon.lineTo(ir.getX()+16,ir.getY()+5);icon.lineTo(ir.getRight()-2,ir.getY()+5);}
    else{icon.addEllipse(ir.getX()+1,ir.getCentreY()-5,10,10);icon.addEllipse(ir.getX()+8,ir.getY()+2,10,10);icon.addEllipse(ir.getX()+8,ir.getBottom()-12,10,10);}
    g.setColour(juce::Colour::fromRGB(26,28,30)); g.strokePath(icon,juce::PathStrokeType(2.1f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    g.setColour(juce::Colour::fromRGB(218,220,219)); g.drawLine((float)r.getX()+14.0f,(float)r.getY()+52.0f,(float)r.getRight()-14.0f,(float)r.getY()+52.0f,1.0f);
}

void PlasmaPercAudioProcessorEditor::paint (juce::Graphics& g)
{
    const float sx = (float) getWidth() / 1672.0f;
    const float sy = (float) getHeight() / 941.0f;
    auto R = [sx, sy] (float x, float y, float w, float h)
    {
        return juce::Rectangle<float> (x * sx, y * sy, w * sx, h * sy);
    };

    g.fillAll (juce::Colour::fromRGB (243, 244, 242));

    juce::ColourGradient bg (juce::Colour::fromRGB (252, 252, 250), 836.0f*sx, 0.0f,
                             juce::Colour::fromRGB (237, 239, 238), 836.0f*sx, 941.0f*sy, false);
    bg.addColour (0.24, juce::Colour::fromRGB (247, 248, 246));
    g.setGradientFill (bg);
    g.fillAll();

    drawPanel (g, R (10, 143, 502, 774).toNearestInt(), "SEQUENCERS");
    drawPanel (g, R (520, 143, 390, 774).toNearestInt(), "VOICE ENGINES");
    drawPanel (g, R (918, 143, 372, 511).toNearestInt(), "ENV / FILTER / GENERATIVE");
    drawPanel (g, R (918, 663, 372, 254).toNearestInt(), "BITCRUSHER (REDUX)");
    drawPanel (g, R (1298, 143, 364, 774).toNearestInt(), "EVERYTHING MACROS");

    g.setColour (juce::Colour::fromRGB (224, 225, 223));
    for (int i = 1; i < 4; ++i)
    {
        const float y = (201.0f + i * 160.0f) * sy;
        g.drawLine (28.0f*sx, y, 497.0f*sx, y, 1.0f);
    }
    for (int i = 1; i < 4; ++i)
    {
        const float y = (201.0f + i * 174.0f) * sy;
        g.drawLine (1312.0f*sx, y, 1648.0f*sx, y, 1.0f);
    }

    // Header: power, arrows and the approved simple utility language.
    g.setColour (juce::Colour::fromRGB (26, 28, 30));
    {
        auto pr = R (31, 76, 38, 38);
        g.drawEllipse (pr.reduced (5.0f*sx), 2.6f*sx);
        g.drawLine (pr.getCentreX(), pr.getY()+2.0f*sy,
                    pr.getCentreX(), pr.getCentreY()+3.0f*sy, 2.8f*sx);
    }
    g.setFont (juce::Font (juce::FontOptions (25.0f*sx)));
    g.drawText ("‹", R (99, 73, 30, 42).toNearestInt(), juce::Justification::centred);
    g.drawText ("›", R (135, 73, 30, 42).toNearestInt(), juce::Justification::centred);

    if (fairyImage.isValid())
    {
        g.setOpacity (0.98f);
        auto logo = R (645, 0, 94, 115).toNearestInt();
        g.drawImageWithin (fairyImage, logo.getX(), logo.getY(), logo.getWidth(), logo.getHeight(),
                           juce::RectanglePlacement::centred, false);

        g.setOpacity (0.17f);
        auto art = R (1452, 188, 198, 646).toNearestInt();
        g.drawImageWithin (fairyImage, art.getX(), art.getY(), art.getWidth(), art.getHeight(),
                           juce::RectanglePlacement::centred, false);
        g.setOpacity (1.0f);
    }

    // The user explicitly asked to keep only PLASMATIK here — no subtitle.
    g.setColour (juce::Colour::fromRGB (18, 20, 23));
    g.setFont (juce::Font (juce::FontOptions (31.0f*sx)));
    g.drawText ("P  L  A  S  M  A  T  I  K",
                R (718, 70, 424, 43).toNearestInt(), juce::Justification::centred);

    // Top right utilities, same spacing as the approved artwork.
    auto drawTopBox = [&] (juce::Rectangle<float> rr)
    {
        g.setColour (juce::Colour::fromRGB (250, 250, 248));
        g.fillRoundedRectangle (rr, 4.0f*sx);
        g.setColour (juce::Colour::fromRGB (201, 203, 202));
        g.drawRoundedRectangle (rr, 4.0f*sx, 1.0f);
    };
    drawTopBox (R (1360, 76, 52, 45));
    g.setColour (juce::Colour::fromRGB (24, 26, 28));
    g.setFont (juce::Font (juce::FontOptions (17.0f*sx)).boldened());
    g.drawText ("◇", R (1362, 77, 48, 43).toNearestInt(), juce::Justification::centred);
    g.setFont (juce::Font (juce::FontOptions (13.0f*sx)).boldened());
    g.drawText ("INIT", R (1429, 80, 48, 34).toNearestInt(), juce::Justification::centred);
    g.drawText ("SAVE", R (1504, 80, 55, 34).toNearestInt(), juce::Justification::centred);
    g.setFont (juce::Font (juce::FontOptions (22.0f*sx)).boldened());
    g.drawText ("•••", R (1581, 78, 54, 34).toNearestInt(), juce::Justification::centred);

    // GLOBAL + settings at the top of the sequencer card.
    {
        auto gb = R (307, 153, 142, 35);
        drawTopBox (gb);
        g.setColour (juce::Colour::fromRGB (26, 28, 30));
        g.setFont (juce::Font (juce::FontOptions (12.5f*sx)).boldened());
        g.drawText ("◇   GLOBAL    ⌄", gb.toNearestInt(), juce::Justification::centred);
        g.setFont (juce::Font (juce::FontOptions (19.0f*sx)).boldened());
        g.drawText ("⚙", R (461, 153, 33, 35).toNearestInt(), juce::Justification::centred);
    }

    // Voice identity dots.
    const std::array<float,4> trackY { 221.0f, 382.0f, 544.0f, 705.0f };
    const std::array<float,4> macroY { 211.0f, 385.0f, 559.0f, 733.0f };
    for (int v = 0; v < 4; ++v)
    {
        g.setColour (voiceColour (v));
        g.fillEllipse (R (37, trackY[(size_t)v], 22, 22));
        g.fillEllipse (R (1310, macroY[(size_t)v], 18, 18));
    }

    // Sequencer footer.
    auto drawMini = [&] (const juce::String& text, juce::Rectangle<float> rr)
    {
        g.setColour (juce::Colour::fromRGB (250, 250, 248));
        g.fillRoundedRectangle (rr, 4.0f*sx);
        g.setColour (juce::Colour::fromRGB (199, 201, 200));
        g.drawRoundedRectangle (rr, 4.0f*sx, 1.0f);
        g.setColour (juce::Colour::fromRGB (29, 31, 33));
        g.setFont (juce::Font (juce::FontOptions (11.5f*sx)).boldened());
        g.drawText (text, rr.toNearestInt(), juce::Justification::centred);
    };
    drawMini ("▶", R (31, 831, 50, 35));
    g.setColour (juce::Colour::fromRGB (29, 31, 33));
    g.setFont (juce::Font (juce::FontOptions (11.5f*sx)).boldened());
    g.drawText ("STEPS", R (93, 831, 54, 35).toNearestInt(), juce::Justification::centredLeft);
    drawMini ("16   ⌄", R (145, 831, 72, 35));
    drawMini ("COPY", R (242, 831, 59, 35));
    drawMini ("PASTE", R (306, 831, 64, 35));
    drawMini ("CLEAR", R (374, 831, 65, 35));
    drawMini ("⤨", R (444, 831, 46, 35));

    // Crusher power + bottom MODE / GEN RATE row from the approved image.
    g.setColour (juce::Colour::fromRGB (25, 27, 29));
    {
        auto pwr = R (1241, 671, 28, 28);
        g.drawEllipse (pwr.reduced (4.0f*sx), 2.0f*sx);
        g.drawLine (pwr.getCentreX(), pwr.getY()+2.0f*sy,
                    pwr.getCentreX(), pwr.getCentreY()+3.0f*sy, 2.1f*sx);
    }

    g.setFont (juce::Font (juce::FontOptions (10.8f*sx)).boldened());
    g.drawText ("MODE", R (932, 844, 48, 22).toNearestInt(), juce::Justification::centredLeft);
    g.drawText ("GEN RATE", R (1146, 844, 73, 22).toNearestInt(), juce::Justification::centredLeft);

    auto modeBox = R (979, 840, 162, 31);
    drawTopBox (modeBox);
    g.setColour (juce::Colour::fromRGB (29, 31, 33));
    g.setFont (juce::Font (juce::FontOptions (10.3f*sx)));
    g.drawText ("Generative Percussion   ⌄", modeBox.toNearestInt(), juce::Justification::centred);

    auto genTrack = R (1217, 850, 57, 9);
    g.setColour (juce::Colour::fromRGB (220, 222, 220));
    g.fillRoundedRectangle (genTrack, 4.5f*sx);
    auto genFill = genTrack; genFill.setWidth (genTrack.getWidth()*0.46f);
    g.setColour (juce::Colour::fromRGB (96, 100, 101));
    g.fillRoundedRectangle (genFill, 4.5f*sx);

    // Bottom master labels; actual sliders sit beside them.
    g.setColour (juce::Colour::fromRGB (28, 30, 32));
    g.setFont (juce::Font (juce::FontOptions (11.8f*sx)).boldened());
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

        s.title.setBounds (B (68, y, 62, 31));
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

        m.title.setBounds (B (1338, y, 138, 27));
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

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MakoBiteAudioProcessorEditor::MakoBiteAudioProcessorEditor (MakoBiteAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{    
    //R1.00 Create SLIDER ATTACHMENTS so our parameter vars get adjusted automatically for Get/Set states.
    ParAtt[e_Gain] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "gain", sldKnob[e_Gain]);
    ParAtt[e_LowCut] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "lowcut", sldKnob[e_LowCut]);           
    ParAtt[e_NGate] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "ngate", sldKnob[e_NGate]);
    ParAtt[e_Drive] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "drive", sldKnob[e_Drive]);
    ParAtt[e_Comp1] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "comp1", sldKnob[e_Comp1]);
    ParAtt[e_Comp2] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "comp2", sldKnob[e_Comp2]);
    ParAtt[e_Low] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "low", sldKnob[e_Low]);
    ParAtt[e_Mid] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "mid", sldKnob[e_Mid]);
    ParAtt[e_High] = std::make_unique <juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters, "high", sldKnob[e_High]);
        
    imgBackground = juce::ImageCache::getFromMemory(BinaryData::precogback01_png, BinaryData::precogback01_pngSize);

    //****************************************************************************************
    //R1.00 Add GUI CONTROLS
    //****************************************************************************************
    Mako_Init_Large_Slider(&sldKnob[e_Gain], audioProcessor.Setting[e_Gain],0.0f, 1.0f,.01f,"", 1, 0xFFE0DACE);
    Mako_Init_Large_Slider(&sldKnob[e_LowCut], audioProcessor.Setting[e_LowCut], 20, 200, 10, "", 1, 0xFF202020);
    Mako_Init_Large_Slider(&sldKnob[e_NGate], audioProcessor.Setting[e_NGate], 0.0f, 1.0f, .01f, "", 1, 0xFF202020);
    Mako_Init_Large_Slider(&sldKnob[e_Drive], audioProcessor.Setting[e_Drive], 0.0f, 1.0f, .01f, "", 1, 0xFFE0DACE);
    Mako_Init_Large_Slider(&sldKnob[e_Comp1], audioProcessor.Setting[e_Comp1], 0.0f, 1.0f, .01f, "", 1, 0xFF202020);
    Mako_Init_Large_Slider(&sldKnob[e_Comp2], audioProcessor.Setting[e_Comp2], 0.0f, 1.0f, .01f, "", 1, 0xFF202020);
    Mako_Init_Large_Slider(&sldKnob[e_Low], audioProcessor.Setting[e_Low], -12.0f, 12.0f, .1f, "", 1, 0xFF202020);
    Mako_Init_Large_Slider(&sldKnob[e_Mid], audioProcessor.Setting[e_Mid], -12.0f, 12.0f, .1f, "", 1, 0xFF202020);
    Mako_Init_Large_Slider(&sldKnob[e_High], audioProcessor.Setting[e_High], -12.0f, 12.0f, .1f, "", 1, 0xFF202020);
    
    //R1.00 Define our control positions to make drawing easier.
    Mako_Knob_DefinePosition(e_LowCut, 10, 60, 50, 50, "LCut");
    Mako_Knob_DefinePosition(e_NGate,  60, 60, 50, 50, "Gate");
    Mako_Knob_DefinePosition(e_Comp1,  110, 50, 40, 40, "Comp");
    Mako_Knob_DefinePosition(e_Comp2,  110, 80, 40, 40, "Comp");
    
    Mako_Knob_DefinePosition(e_Gain, 175, 55, 70, 70, "Gain");
    Mako_Knob_DefinePosition(e_Drive, 245, 55, 70, 70, "Drive");
    
    Mako_Knob_DefinePosition(e_Low, 330, 60, 50, 50, "Low");
    Mako_Knob_DefinePosition(e_Mid, 380, 60, 50, 50, "Mid");
    Mako_Knob_DefinePosition(e_High, 430, 60, 50, 50, "High");

    Knob_Cnt = 9;

    //R2.00 Start our Timer so we can tell the user they are clipping. Could draw VU Meters here, etc.
    startTimerHz(10);  //R1.00 have our Timer get called 10 times per second.

    //R1.00 Update the Look and Feel (Global colors) so drop down menu is the correct color. 
    getLookAndFeel().setColour(juce::DocumentWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::DocumentWindow::textColourId, juce::Colour(255, 255, 255));
    getLookAndFeel().setColour(juce::DialogWindow::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonOnColourId, juce::Colour(192, 0, 0));
    getLookAndFeel().setColour(juce::TextButton::buttonColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ComboBox::backgroundColourId, juce::Colour(0, 0, 0));
    getLookAndFeel().setColour(juce::ListBox::backgroundColourId, juce::Colour(32, 32, 32));
    getLookAndFeel().setColour(juce::Label::backgroundColourId, juce::Colour(32, 32, 32));
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //R1.00 Set the window size.
    setSize(490, 130);
}

MakoBiteAudioProcessorEditor::~MakoBiteAudioProcessorEditor()
{
}

//==============================================================================
void MakoBiteAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    
    bool UseImage = true;
    juce::ColourGradient ColGrad;

    if (UseImage)
    {
        g.drawImageAt(imgBackground, 0, 0);        
    }
    else
    {
        //R1.00 Draw our GUI.
        //R1.00 Background.
        g.setColour(juce::Colour(0xFFFFFFFF));
        g.fillRect(0, 0, 490, 130);
        
        //R1.00 Draw LOGO text.
        g.setColour(juce::Colour(0xFF404040));
        g.fillRect(185, 0, 120, 35);
        g.setFont(16.0f);
        g.setColour(juce::Colours::white);
        g.drawFittedText("P R E C O G", 185, 0, 120, 18, juce::Justification::centred, 1);
        g.setFont(14.0f);
        g.setColour(juce::Colour(0xFF80C0FF));
        g.drawFittedText("m a k o", 185, 15, 120, 15, juce::Justification::centred, 1);

        //R1.00 Draw Slider TEXT.
        g.setFont(12.0f);
        g.setColour(juce::Colours::black);
        for (int t = 0; t < Knob_Cnt; t++)
        {
            g.drawFittedText(Knob_Name[t], Knob_Pos[t].x, Knob_Pos[t].y - 10, Knob_Pos[t].sizex, 15, juce::Justification::centred, 1);
        }

        g.setColour(juce::Colours::black);

        //R1.00 LEFT VU Meter area.
        g.fillRect(10, 10, 155, 20);
        g.fillEllipse(170, 20, 10, 10);

        //R1.00 RIGHT VU Meter area.
        g.fillRect(325, 10, 155, 20);
        g.setColour(juce::Colours::black);
        g.fillEllipse(310, 20, 10, 10);

        //R1.00 Draw additional UI text.
        g.setColour(juce::Colours::black);
        g.drawFittedText("Left Channel", 10, 32, 155, 15, juce::Justification::centredLeft, 1);
        g.drawFittedText("ov", 165, 10, 20, 10, juce::Justification::centred, 1);
        g.drawFittedText("Right Channel", 325, 32, 155, 15, juce::Justification::centredRight, 1);
        g.drawFittedText("ov", 305, 10, 20, 10, juce::Justification::centred, 1);
    }    
    
    //R1.00 Draw the Compression indicator LED and Limit Line.
    if (audioProcessor.Setting[e_Comp1] < 1.0f)
    {
        //R1.00 Limit Line.
        g.setColour(juce::Colour(0xFF0080B0));
        int Coff = audioProcessor.Setting[e_Comp1] * 150;
        g.drawLine(13 + Coff, 12, 13 + Coff, 30, 2.0f);
        g.drawLine(328 + Coff, 12, 328 + Coff, 30, 2.0f);

        //R1.00 Indicator LED.
        if ((audioProcessor.Setting[e_Comp1] < audioProcessor.VUValue[2]) || (audioProcessor.Setting[e_Comp1] < audioProcessor.VUValue[3]))
        {
            g.setColour(juce::Colour(0xFF00E0FF));
            g.fillEllipse(150, 50, 6, 6);
        }
    }

    //**********************************************
    //R1.00 LEFT VU Meter bar
    //**********************************************
    g.setColour(juce::Colour(0xFF00C0B0));
    g.fillRect(13, 15, int(150 * VULast[0] * .01f), 2);
    
    ColGrad = juce::ColourGradient(juce::Colour(0xFF00FFC0), 10.0f, 0.0f, juce::Colour(0xFFFF0000), 195.0f, 0.0f, false);
    g.setGradientFill(ColGrad);
    g.fillRect(13, 21, int(150 * VULast[2] * .01f), 6);
    
    //R1.00 If clipping draw the OverLoad LED on. Clip count will be a number above 0.  
    if (ClipCount[2])
    {
        g.setColour(juce::Colours::red);
        g.fillEllipse(172, 22, 6, 6);        
    }
    
    //**********************************************
    //R1.00 RIGHT VU Meter bar.
    //**********************************************
    g.setColour(juce::Colour(0xFF00C0B0));
    g.fillRect(328, 15, int(150 * VULast[1] * .01f), 2);

    ColGrad = juce::ColourGradient(juce::Colour(0xFF00FFC0), 325.0f, 0.0f, juce::Colour(0xFFFF0000), 520.0f, 0.0f, false);
    g.setGradientFill(ColGrad);
    g.fillRect(328, 21, int(150 * VULast[3] * .01f), 6);
        
    //R1.00 If clipping draw the OverLoad LED on. Clip count will be a number above 0.  
    if (ClipCount[3])
    {
        g.setColour(juce::Colours::red);
        g.fillEllipse(312, 22, 6, 6);
    }

}

void MakoBiteAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    //R1.00 Define positions for all of our KNOBS.
    for (int t = 0; t < Knob_Cnt; t++) sldKnob[t].setBounds(Knob_Pos[t].x, Knob_Pos[t].y, Knob_Pos[t].sizex, Knob_Pos[t].sizey);    
}


//R1.00 Setup the SLIDER control edit values, Text Suffix (if any), UI tick marks, and Indicator Color.
void MakoBiteAudioProcessorEditor::Mako_Init_Large_Slider(juce::Slider* slider, float Val, float Vmin, float Vmax, float Vinterval, juce::String Suffix, int TickStyle, int ThumbColor)
{
    //R1.00 Setup the slider edit parameters.
    slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    slider->setTextValueSuffix(Suffix);
    slider->setRange(Vmin, Vmax, Vinterval);
    slider->setValue(Val);
    slider->addListener(this);
    addAndMakeVisible(slider);

    //R1.00 Override the default Juce drawing routines and use ours.
    slider->setLookAndFeel(&myLookAndFeel);

    //R1.00 Setup the type and colors for the sliders.
    slider->setSliderStyle(juce::Slider::SliderStyle::Rotary);
    slider->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xFFC08000));
    slider->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF000000));
    slider->setColour(juce::Slider::textBoxHighlightColourId, juce::Colour(0xFF804000));
    slider->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0x00000000));    //R1.00 Make this SEE THRU. Alpha=0.
    slider->setColour(juce::Slider::thumbColourId, juce::Colour(ThumbColor));

    //R1.00 Cheat: We are using this color as a Tick Mark style selector in our drawing function.
    slider->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(TickStyle));
}

//R1.00 Store information about each knob, like size, title, etc.
void MakoBiteAudioProcessorEditor::Mako_Knob_DefinePosition(int idx,float x, float y, float sizex, float sizey, juce::String name)
{
    Knob_Pos[idx].x = x;
    Knob_Pos[idx].y = y;
    Knob_Pos[idx].sizex = sizex;
    Knob_Pos[idx].sizey = sizey;
    Knob_Name[idx] = name;
}

//R1.00 This gets called when a knob or slider ar adjusted.
void MakoBiteAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{  
    //R1.00 When a slider is adjusted, this func gets called. Capture the new edits and flag
    //R1.00 the processor when it needs to recalc things.
    //R1.00 Check which slider has been adjusted.
    for (int t = 0; t < Knob_Cnt; t++)
    {
        if (slider == &sldKnob[t])
        {            
            //R1.00 Update the actual processor variable being edited.
            audioProcessor.Setting[t] = float(sldKnob[t].getValue());

            //R1.00 We need to update settings in processor.
            //R1.00 Increment changed var to be sure every change gets made. Changed var is decremented in processor.
            audioProcessor.SettingsChanged += 1;

            if (t == e_Comp1) repaint();

            //R1.00 We have captured the correct slider change, exit this function.
            return;
        }
    }
    
    return;
}

//R1.00 This timer gets called to update our UI VU meters.
//R1.00 Redrawing the UI is very CPU heavy so we are trying to only REDRAW when something has changed.
//R1.00 We convert our VU value to 0-100 integer to track changes easier and reduce draws.
void MakoBiteAudioProcessorEditor::timerCallback()
{
    int tUV[4];
    bool Redraw = false;

    //R1.00 loop thru our Input/Output VU values.
    for (int t = 0; t < 4; t++)
    {
        tUV[t] = int(audioProcessor.VUValue[t] * 100);
        if (tUV[t] != VULast[t])
        {
            VULast[t] = tUV[t];
            Redraw = true;
        }

        //R1.00 We are clipping. Set clipcount so the OV LED stays lit for about a second.
        if (.99f < audioProcessor.VUValue[t])
        {
            ClipCount[t] = 11;
            Redraw = true;
        }

        //R1.00 Reset out peak value or gradually decrease the peak value.
        audioProcessor.VUValue[t] = 0.0f; // *= 0.5f;

        //R1.00 Countdown our LEFT CLIP/OV indicator. 
        //R1.00 If the indicator needs changed, set REDRAW to true.
        ClipCount[t]--;
        if (ClipCount[t] < 0) ClipCount[t] = 0;
        if (ClipCount[t])
            Clipping[t] = true;
        else
            Clipping[t] = false;
        if (Clipping[t] != Clipping_Last[t]) Redraw = true;
        Clipping_Last[t] = Clipping[t];
    }


    //R1.00 If we had a value change, we need to redraw the screen.
    if (Redraw) repaint();    
}

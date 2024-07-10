/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "cmath"              //R1.00 Added library.

//==============================================================================
MakoBiteAudioProcessor::MakoBiteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
    ),
    
    //R1.00 Define our VALUE TREE parameter variables. Min val, Max Val, default Val.
    parameters(*this, nullptr, "PARAMETERS", 
      {        
        std::make_unique<juce::AudioParameterInt>("lowcut","Low Cut", 20, 200, 10),
        std::make_unique<juce::AudioParameterFloat>("ngate","Noise Gate", .0f, 1.0f, .0f),
        std::make_unique<juce::AudioParameterFloat>("comp1","Comp Thresh", 0.0f, 1.0f, 1.0f),
        std::make_unique<juce::AudioParameterFloat>("comp2","Comp Ratio", 0.0f, 1.0f, 1.0f),
        
        std::make_unique<juce::AudioParameterFloat>("gain","Gain", .0f, 1.0f, .3162278f),
        std::make_unique<juce::AudioParameterFloat>("drive","Drive", .0f, 1.0f, .0f),        
        
        std::make_unique<juce::AudioParameterFloat>("low","Low", -12.0f, 12.0f, .0f),
        std::make_unique<juce::AudioParameterFloat>("mid","Mid", -12.0f, 12.0f, .0f),
        std::make_unique<juce::AudioParameterFloat>("high","High", -12.0f, 12.0f, .0f),
      }
    )   

#endif
{   
}

MakoBiteAudioProcessor::~MakoBiteAudioProcessor()
{
}

//==============================================================================
const juce::String MakoBiteAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MakoBiteAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MakoBiteAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MakoBiteAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MakoBiteAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MakoBiteAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MakoBiteAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MakoBiteAudioProcessor::getProgramName (int index)
{
    return {};
}

void MakoBiteAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MakoBiteAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //R1.00 Get our Sample Rate for filter calculations.
    SampleRate = MakoBiteAudioProcessor::getSampleRate();
    if (SampleRate < 21000) SampleRate = 48000;
    if (192000 < SampleRate) SampleRate = 48000;

    //R1.00 Calculate some rough decay subtraction values for peak tracking (compress,autowah,etc). 
    Release_5mS = (1.0f / .005f) * (1.0f / SampleRate);
    Release_10mS = (1.0f / .010f) * (1.0f / SampleRate);
    Release_50mS = (1.0f / .05f) * (1.0f / SampleRate);
    Release_100mS = (1.0f / .100f) * (1.0f / SampleRate);
    Release_200mS = (1.0f / .200f) * (1.0f / SampleRate);
    Release_300mS = (1.0f / .300f) * (1.0f / SampleRate);  
    Release_400mS = (1.0f / .400f) * (1.0f / SampleRate); 
    Release_500mS = (1.0f / .500f) * (1.0f / SampleRate); 

    //R1.00 Update the adjustable values and filters. 
    Mako_Settings_Update(true);
}

void MakoBiteAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MakoBiteAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MakoBiteAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //R1.00 Our defined variables.
    float tS;  //R1.00 Temporary Sample.

    //R1.00 Handle any changes to our Parameters made in the editor/DAW.
    if (0 < SettingsChanged) Mako_Settings_Update(false);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        
        // ..do something to the data...
        for (int samp = 0; samp < buffer.getNumSamples(); samp++)
        {
            //R1.00 Get the current sample and put it in tS. 
            tS = buffer.getSample(channel, samp);

            //R1.00 Track our loudest INPUT signal.
            if (VUValue[channel] < abs(tS)) VUValue[channel] = abs(tS);

            //R1.00 Apply Low Cut Filter if being used.
            if (20.0f < Setting[e_LowCut]) tS = Filter_Calc_BiQuad(tS, channel, &makoF_LowCut);

            //R1.00 Apply Noise gate if being used.
            if (0.0f < Setting[e_NGate]) tS = Mako_FX_NoiseGate(tS, channel);

            //R1.00 Apply our EQ and gain to the sample. 
            tS = Mako_FX_EQandGain(tS, channel);

            //R1.00 Compressor. Could be here or before gain.
            if (Setting[e_Comp1] < 1.0f) tS = Mako_FX_Compressor(tS, channel);
            
            //R1.00 Clip and track the loudest OUTPUT signal so far.
            if (1.0f < tS) tS = 1.0f;
            if (tS < -1.0f) tS = -1.0f;
            if (VUValue[channel + 2] < abs(tS)) VUValue[channel + 2] = abs(tS);

            //R1.00 Write our modified sample into the buffer.
            channelData[samp] = tS;

        }
       
    }
}

//==============================================================================
bool MakoBiteAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MakoBiteAudioProcessor::createEditor()
{
    return new MakoBiteAudioProcessorEditor (*this);
}


//==============================================================================
void MakoBiteAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    //R1.00 Save our parameters to file/DAW.
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
   
}

void MakoBiteAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    //R1.00 Read our parameters from file/DAW.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));

    //R1.00 Force our variables to get updated.
    Setting[e_Gain] = Mako_GetParmValue_float("gain");
    Setting[e_LowCut] = Mako_GetParmValue_float("lowcut");
    Setting[e_NGate] = Mako_GetParmValue_float("ngate");    
    Setting[e_Drive] = Mako_GetParmValue_float("drive");
    Setting[e_Comp1] = Mako_GetParmValue_float("comp1");
    Setting[e_Comp2] = Mako_GetParmValue_float("comp2");
    Setting[e_Low] = Mako_GetParmValue_float("low");
    Setting[e_Mid] = Mako_GetParmValue_float("mid");
    Setting[e_High] = Mako_GetParmValue_float("high");
}

//R1.00 Parameter reading helper function.
int MakoBiteAudioProcessor::Mako_GetParmValue_int(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return int(parm->load());
    else
        return 0;
}

//R1.00 Parameter reading helper function.
float MakoBiteAudioProcessor::Mako_GetParmValue_float(juce::String Pstring)
{
    auto parm = parameters.getRawParameterValue(Pstring);
    if (parm != NULL)
        return float(parm->load());
    else
        return 0.0f;
}

//R1.00 Volume envelope based on average Signal volume.
float MakoBiteAudioProcessor::Mako_FX_NoiseGate(float tSample, int channel)
{
    //R1.00 Track our Input Signal Average (Absolute vals).
    Signal_AVG[channel] = (Signal_AVG[channel] * .995) + (abs(tSample) * .005);

    //R1.00 Create a volume envelope based on Signal Average.
    Pedal_NGate_Fac[channel] = Signal_AVG[channel] * 10000.0f * (1.1f - Setting[e_NGate]);

    //R1.00 Dont amplify the sound, just reduce when necessary.
    if (1.0f < Pedal_NGate_Fac[channel]) Pedal_NGate_Fac[channel] = 1.0f;

    return tSample * Pedal_NGate_Fac[channel];
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MakoBiteAudioProcessor();
}

//R1.00 Apply filter to a sample.
float MakoBiteAudioProcessor::Filter_Calc_BiQuad(float tSample, int channel, tp_filter* fn)
{
    float tS = tSample;

    fn->xn0[channel] = tS;
    tS = fn->a0 * fn->xn0[channel] + fn->a1 * fn->xn1[channel] + fn->a2 * fn->xn2[channel] - fn->b1 * fn->yn1[channel] - fn->b2 * fn->yn2[channel];
    fn->xn2[channel] = fn->xn1[channel]; fn->xn1[channel] = fn->xn0[channel]; fn->yn2[channel] = fn->yn1[channel]; fn->yn1[channel] = tS;

    return tS;
}

//R1.00 Second order parametric/peaking boost filter with constant-Q
void MakoBiteAudioProcessor::Filter_BP_Coeffs(float Gain_dB, float Fc, float Q, tp_filter* fn)
{    
    float K = pi2 * (Fc * .5f) / SampleRate;
    float K2 = K * K;
    float V0 = pow(10.0, Gain_dB / 20.0);

    float a = 1.0f + (V0 * K) / Q + K2;
    float b = 2.0f * (K2 - 1.0f);
    float g = 1.0f - (V0 * K) / Q + K2;
    float d = 1.0f - K / Q + K2;
    float dd = 1.0f / (1.0f + K / Q + K2);

    fn->a0 = a * dd;
    fn->a1 = b * dd;
    fn->a2 = g * dd;
    fn->b1 = b * dd;
    fn->b2 = d * dd;
    fn->c0 = 1.0f;
    fn->d0 = 0.0f;
}

//R1.00 Second order butterworth LOW PASS filter. 
void MakoBiteAudioProcessor::Filter_LP_Coeffs(float fc, tp_filter* fn)
{    
    float c = 1.0f / (tanf(pi * fc / SampleRate));
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = 2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * (1.0f - (c * c));
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

//R1.00 Second order butterworth HIGH PASS filter.
void MakoBiteAudioProcessor::Filter_HP_Coeffs(float fc, tp_filter* fn)
{    
    float c = tanf(pi * fc / SampleRate);
    fn->a0 = 1.0f / (1.0f + sqrt2 * c + (c * c));
    fn->a1 = -2.0f * fn->a0;
    fn->a2 = fn->a0;
    fn->b1 = 2.0f * fn->a0 * ((c * c) - 1.0f);
    fn->b2 = fn->a0 * (1.0f - sqrt2 * c + (c * c));
}

//R1.00 Apply some EQ and gain to the sample.
float MakoBiteAudioProcessor::Mako_FX_EQandGain(float tSample, int channel)
{
    float tS = tSample;
    
    //R1.00 Apply our 3-band EQ to the signal.
    if (0.0f != Setting[e_Low]) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Low);
    if (0.0f != Setting[e_Mid]) tS = Filter_Calc_BiQuad(tS, channel, &makoF_Mid);
    if (0.0f != Setting[e_High]) tS = Filter_Calc_BiQuad(tS, channel, &makoF_High);

    //R1.00 Apply some gain/drive/distortion.
    if (0.0f < Setting[e_Drive]) tS = tanhf(tS * (.1f + Setting[e_Drive]) * 6.0f);

    //R1.00 Volume/Gain adjust.
    return Setting[e_Gain] * Setting[e_Gain] * tS * 10.0f;
}

//R1.00 MAKO COMPRESSOR - Try to limit guitar dynamic range.
float MakoBiteAudioProcessor::Mako_FX_Compressor(float tSample, int channel)
{
    //R1.00 The compressor needs Threshold, Ratio, Attack, and Release vars.
    //R1.00 We are using fixed vals for all but Ratio. Should be more knobs on screen.
    //R1.00 Could add advanced features like increase Release time if we are in compression for a long time.
    float diff;
    float tSa = abs(tSample);
    float Thresh = Setting[e_Comp1];  //R1.00 Compressor Threshold.
    float Ratio = Setting[e_Comp2];   //R1.00 Compressor RATIO.

    //R1.00 If our signal is above the Threshold we need to start compressing.
    if (Thresh < tSa)
    {
        //R1.00 Get Difference in Gain and Threshold.    
        diff = tSa - Thresh;

        //R1.00 Calc what our new gain reduction value should be.
        Pedal_CompGain[channel] = (Thresh + (diff * Ratio)) / tSa;

        //R1.00 Slowly modify our GAIN adjuster to the new gain value. 
        if (Pedal_CompGain[channel] < Pedal_CompGainAdj[channel])
        {
            //R1.00 To have a comp attack we need a 2nd var that we adjust up to the actual max. So the comp slowly begins working.
            //R1.00 ATTACK - Slowly reduce the gain to the desired value.
            Pedal_CompGainAdj[channel] -= Release_5mS; 
            if (Pedal_CompGainAdj[channel] < 0.0f) Pedal_CompGainAdj[channel] = 0.0f;
        }
        else
        {
            //R1.00 RELEASE - Adjust the gain back up to 1.0f.
            Pedal_CompGainAdj[channel] += Release_50mS;
            if (1.0f < Pedal_CompGainAdj[channel]) Pedal_CompGainAdj[channel] = 1.0f;
        }
    }
    else
    {
        //R1.00 Signal is BELOW the threshold, stop compressing and RELEASE - Adjust the gain back up to 1.0f.
        Pedal_CompGainAdj[channel] += Release_50mS;
        if (1.0f < Pedal_CompGainAdj[channel]) Pedal_CompGainAdj[channel] = 1.0f;
    }

    return tSample * Pedal_CompGainAdj[channel];    
}


void MakoBiteAudioProcessor::Mako_Settings_Update(bool ForceAll)
{
    //R1.00 We do changes here so we know the vars are not in use while we change them.
    //R1.00 EDITOR sets SETTING flags and we make changes here.
    //R1.00 If there are any settings
    bool Force = ForceAll;

    //R1.00 Update our EQ Filters.
    Filter_HP_Coeffs(Setting[e_LowCut], &makoF_LowCut);
    Filter_BP_Coeffs(Setting[e_Low], 450.0f, .707f, &makoF_Low);
    Filter_BP_Coeffs(Setting[e_Mid], 750.0f, .707f, &makoF_Mid);
    Filter_BP_Coeffs(Setting[e_High], 1500.0f, .707f, &makoF_High);

    //R1.00 RESET out settings flags.
    SettingsType = 0;
    SettingsChanged = false;
}



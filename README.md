# MakoPrecog
* A demo JUCE VST guitar preamplifier and conditioner
* Sample code.
* Tested on Windows only.
* Written in Visual C++ 2022.
* Written for new programmers, not complicated.
* Version: 1.00
* Posted: July 10, 2024

VERSION
------------------------------------------------------------------
1.00 - Initial release.  

DISCLAIMER
------------------------------------------------------------------  
This VST was written in a very simple way. No object defs, etc. 
Just simple code for people who are not programmers but want to 
dabble in the magic of JUCE VSTs.

If you are new to JUCE code, the Mako Thump VST may be a better
starting place. Or be brave and try this.
       
SUMMARY
------------------------------------------------------------------
A Juce/C++ VST3 written to allow users to finetune a guitar signal
for use with other amplifier VSTs.

![Demo Image](docs/assets/precogdemo.jpg)

# THEORY OF OPERATION<br />
This VST is designed to mimic a low gain guitar amplifier. It applies an EQ to the guitar signal before it
applies any gain. 

The guitar signal chain thru the VST is:  
Guitar -> Noise Gate -> EQ -> Gain -> High Cut -> Impulse Response Speaker Sim -> Compressor

EQ BEFORE GAIN  
Placing the EQ section before the gain stages gives you full control of the distortion voice. This works best for low to medium gain amplifiers.
Excessive bass and treble into a high gain stage can sound very bad.  

NOTE: Since our EQ circuit adds volume, it can be used to boost distortion.

COMPRESSOR  
Since the VST is trying to duplicate and older style cleanish amp, a compressor was added to bring out the
jingle and allow for close to breakup sounds. 

The single control is for the compressor THRESHOLD. This is the volume the incoming signal must hit to trigger the
compressor. 

A setting of 1.0 (Full On) means the compressor is off and not being used.  


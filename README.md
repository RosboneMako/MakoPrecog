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

![Demo Image](docs/assets/precogdemo.png)

# THEORY OF OPERATION<br />
This VST is designed to accompany and enhance other amplifier VSTs.

The guitar signal chain thru the VST is:  
Guitar -> Low Cut -> Noise Gate -> EQ -> Gain -> Compressor

There are many new amplifier VSTs out that rely on user created amplfier profiling. These VSTs can be limited to the fixed state of the profile.

LOW CUT  
This VST will let the user reduce the lows entering the next VST. This helps clean up and reduce boominess.

3 BAND EQ  
A simple EQ is added to enhance certain frequencies entering an amplifier.
* 450 Hz - Thickens the guitar tone.
* 750 Hz - Typical frequency pushed by gain pedals.
* 1500 Hz - Used to add some more bite.

NOTE: Since our EQ circuit adds volume, it can be used to boost distortion.

COMPRESSOR  
A simple Compressor was added to enhance pick attack. This has a fixed attack time of 5 mS. It has two normal compressor adjustments:
* Threshold - Sets the signal level where the compressor should kick in.
* Ratio - How much volume reduction to apply when the threshold is crossed.

A setting of 1.0 (Full On) means the compressor is off and not being used.  


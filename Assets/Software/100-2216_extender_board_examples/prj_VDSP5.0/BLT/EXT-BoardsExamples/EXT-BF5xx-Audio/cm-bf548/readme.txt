_______________________________________________________________________________

EXT-BF5xx-Audio C Based AD1836 TALKTHRU
______________________________________________________________________________

This directory contains an example ADSP-BF5xx subroutine that demonstrates the 
initialization of the link between SPORT0 on the ADSP-BF5xx and the AD1836 
stereo Codec.  This is done by initializing the link in TDM mode and implementing
a simple talk-through routine.  

Files contained in this directory:

readme.txt				this file
main.c					C file containing the main program and variable declaration
Initialize.c			C file containing all initialization routines
ISR.c					C file containing the interrupt service routine for 
						SPORT0_RX
Process_data.c			C file for processing incoming data
Talkthrough.h			C header file containing prototypes and macros
C_Talkthrough_TDM_xxx.dpj	VisualDSP++ project file
_______________________________________________________________________________


CONTENTS

I.		FUNCTIONAL DESCRIPTION
II.		IMPLEMENTATION DESCRIPTION
III.	OPERATION DESCRIPTION


I.    FUNCTIONAL DESCRIPTION

The Talkthru demo demonstrates the initialization of SPORT0 to establish a link 
between the ADSP-BF5xx and the AD1836 Codec.
 
The program simpley sets up the SPI port to configure the AD1836 codec to TDM mode.
SPORT0 is then setup to receive/transmit audio samples from the AD1836.  Audio
samples received from the AD1836 are moved into the DSP's receive buffer, using 
DMA.  The samples are processed by the ADSP-BF5xx and place in the transmit buffer.
In turn, the transmit buffer is used to transmit data to the AD1836.  This results
in a simple talk-through where data is moved in and out of the DSP with out 
performing any computations on the data.

From one stereo input channel pair the audio stream is moved to all stereo
output channles. With the button on the base-board you can alternate the
input channel pair used.

II.   IMPLEMENTATION DESCRIPTION

The Initialization module initializes:

1. EBIU setup 
3. SPI setup to initialize AD1836 codec
4. SPORT0 setup
5. DMA setup
6. Interrupt configuration


III.  OPERATION DESCRIPTION

- Open the project "C_Talkthrough_TDM_xxx.dpj" in the VisualDSP Integrated Development
	Environment (IDDE) according the Core Module you use.
- Under the "Project" tab, select "Build Project" (program is then loaded 
	automatically into DSP).
- Setup an input source (such as a radio) to the audio in jack and an output 
	source (such as headphones) to the audio out jack of the EXT-BF5xx-Audio.  See the
	EXT-BF5xx-Audio Hardware User Manual for more information on setting up the
	hardware.
- Select "Run" from the "Debug" tab on the menu bar of VisualDSP.
- Listen to the operation of the talk-through

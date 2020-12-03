Project: EXT-BF537e-AD-DA

DIP switch positions:
	EXT-BF5xx-AD-DA Board:
		S100: 	1, 3, 5, 7 OFF
				2, 4, 6, 8 ON
				(see EXT-BF5xx-AD-DA_HardwareUserManual for more information)
				
		S101:	1 ON  --> single ended mode, values are straight binary
				2 OFF --> 2,5V reference voltage, so input has to be between 0V and 2,5V (for AD7266)
						  (if this switch is ON the values are in twos complement, not straight binary)
				3 ON  --> AMS2 is used for communication with AD5405
				4 OFF
				
		S200: 0: Output channel A of AD5405 is routed to connector X200
			  1: Output channel A of AD5415 is routed to connector X200
			  
		S201: 0: Output channel B of AD5405 is routed to connector X200
			  1: Output channel B of AD5415 is routed to connector X200
	
	Base Board:
		For information about the DIP switch settings see the HardwareUserManual of the specific board.


Functional Description:
	AD7266:
		The AD7266 is a 12 bit analog to digital converter with serial interface.
		Max. Rates:
			Sampling Rate: 2MSps
			Serial Clock: 32 MHz
			2 Channels are transmitted in parallel
	
	AD5415:
		The AD5415 is a 12 bit digital to analog converter with serial interface.
		Max. Rates:
			Sampling Rate: 2,47 MSps
			Serial Clock: 50 MHz
			one channel is transmitted after the other (each channel data has information on which channel it has to be outputted)
			in dual channel mode the maximum sampling rate is 1,3 MSps per channel
	
	AD5405:
		The AD5405 is a 12 bit digital to analog converter with parallel interface.
		Max. Rates:
			Sampling Rate: 21,3 MSps
			the real maximal sampling rate on this board is about 200 kSps per channel
	
	
	Loop Through:
		in Environment.h:
			uncomment:
				USE_AD7266_SERIAL
				USE_AD5415_SERIAL
				USE_PROCESS_DATA if the sampled data should be processed
		
			configure the section Global settings according to your needs
			the maximum sampling rate in loop through mode is 1,3 MSps if stride is 1
			and 1 MSps if stride is higher
			
			for good results leave the section crystal frequency settings as they are
		
		on the Extender Board:
			set S200 and S201 to 1
			
			If channel is set to 1 the analog signals has to be applied to X100 Pin 1 (Va1, Channel A) and X106 Pin 1 (Vb1, Channel B).
			(see EXT-BF5xx-AD-DA_HardwareUserManual for more information)
			
			The output of the sampled and resampled signal is on connector X200 for Channel A and X201 on Channel B.
	
	Signal Sample output:
		in Environment.h:
			uncomment:
				USE_AD7266_SERIAL (for activating the ADC and generating the reference voltage for the DAC)
				USE_AD5415_SERIAL (if you want to use the parallel DAC uncomment that, but for higher sampling rates use the serial DAC)
				
			configure the sampling rate as needed (Sampling Rate / 36 is the frequency of the output signal)
		
		on the Extender Board:
			set S200 and S201 according to the used DAC
	
	Display the sampled Data in the memory of the blackfin:
		select the components to use
		
		the adresses of the buffer:
			AD7266:
				aAD7266Configuration.pBufferBase0
				aAD7266Configuration.pBufferBase1
				
				count: aAD7266Configuration.nMaxElements * 2 (for 2 channels)
				datatype: unsigned short
				consider the settings for stride and offset when displaying in the plot window of VisualDSP++
			
			AD5415:
				aAD5415Configuration.pBufferBase0
				aAD5415Configuration.pBufferBase1
				
				count: aAD5415Configuration.nMaxElements ( * 2 for the second channel, depends on the settings)
				datatype: unsigned short
				consider the settings for stride and offset when displaying in the plot window of VisualDSP++
			
			AD5405:
				g_aAD5405BufferSpec[0].pBufferBase
				g_aAD5405BufferSpec[1].pBufferBase
				
				count: aAD5405Configuration.nMaxElements
				datatype: unsigned short
				consider the settings for stride and offset when displaying in the plot window of VisualDSP++
				
(c) Bluetechnix 2009

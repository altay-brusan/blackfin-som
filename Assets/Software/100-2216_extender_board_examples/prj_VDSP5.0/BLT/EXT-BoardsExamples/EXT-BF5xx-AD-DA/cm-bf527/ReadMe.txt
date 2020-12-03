Project: EXT-BF527-AD-DA

DIP switch positions:
	EXT-BF5xx-AD-DA Board:
		S101:	3 ON  --> AMS2 is used for communication with AD5405
				4 OFF
				
		S200: 0: Output channel A of AD5405 is routed to connector X200
			  
		S201: 0: Output channel B of AD5405 is routed to connector X200
	
	Base Board:
		For information about the DIP switch settings see the HardwareUserManual of the specific board.


Functional Description:
	AD5405:
		The AD5405 is a 12 bit digital to analog converter with parallel interface.
		Max. Rates:
			Sampling Rate: 21,3 MSps
			the real maximal sampling rate on this board is about 200 kSps per channel
	
	
	Signal Sample output:
		in Environment.h:
			uncomment:
				USE_AD7266_SERIAL (for activating the ADC and generating the reference voltage for the DAC)
				USE_AD5405_SERIAL
				
			configure the sampling rate as needed (Sampling Rate / 36 is the frequency of the output signal) but consider the max. sampling rate
		
	
	Display the sampled Data in the memory of the blackfin:
		select the components to use
		
		the adresses of the buffer:
			AD5405:
				g_aAD5405BufferSpec[0].pBufferBase
				g_aAD5405BufferSpec[1].pBufferBase
				
				count: aAD5405Configuration.nMaxElements
				datatype: unsigned short
				consider the settings for stride and offset when displaying in the plot window of VisualDSP++
				
(c) Bluetechnix 2009

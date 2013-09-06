WiH2O - Opensource WiFi-enabled Lawn Sprinkler Controller

Introduction:
-------------
This project is a result of my personal effort to create a WiFi-enabled lawn sprinkler controller for my home.

Currenty, the chipKIT µc32 with a chipKIT WiFi shield are required, along with an SD card plugged into the slot on the WiFi
shield, which holds the web pages and the persisted schedule (in order to survive power outages).

The chipKIT µc32 is an Arduino-compatible microcontroller board based on the PIC32 processor made by Digilent. I went with the µc32 because I needed more RAM to hold the schedules. 
The chipKIT µc32 is also quite a bit faster, which is probably a good thing - even with the faster speed, serving up the
web pages is not exactly lightning fast. I suspect a MAX32 would also do the job - and would provide an opportunity to persist
the schedule in EPROM instead of on the SD card. Other WiFi shields might also be compatible.

The software consists of a sketch and related modules developed in the chipKIT MPIDE (available for download on Digilent's
website). This is a version of the Arduino IDE which is compatible with the chipKIT product line. An significant portion of
the project is code which implements a very basic webserver. Implementing an HTTP server from the ground up in c probably would
have been beyond my capability; fortunately, Digilent's Gene Apperson has written a basic implementation which I was able
to use as a starting point. Since starting the project, I've been in touch with Gene, and subsequently Keith Vogel, at 
Digilent. Keith has put together a somewhat more robust version of the HTTP server which would really be better to use...
I just haven't had time to go back an re-implement on that foundation. 

One of two varieties of Real Time Clock can be used (see App.h settings, below). You can either use RTC that is embedded in the
chipKIT PIC32's, or a DS1302-based daughter board connected via 

Requirements:
-------------

	** TODO **
	
	
	
Configuration:
--------------
	
	** TODO - finish **

	In App.h, define these values:
		#define MAX_EVENTS			15		// Number of events to support - must have corresponding initializations lines
											// for zones array in App.cpp
											
		#define FIRST_VALVE_PIN		26		// Defines the I/O pin of the first sprinkler valve solenoid - this pin, followed
											// by the next 5 will control your sprinkler valves
											
		#define MASTER_VALVE_PIN	32		// If you have a master/backflow valve, this pin  will be used to control it.

		#define INVERT_PINS 		'Y' 	// If you want Arduino pin to be HIGH for Off, set to 'Y', normally set to 'N'
											// (my triac board uses reverse levels)
		#define INVERT_MASTER_PIN	'Y'		// Like INVERT_PINS, but for the master valve

		// To specify which Real Time Clock you will use, specify one of either:
		#define   RTCC_DS1302
		//	or:
		#define   RTCC_PIC32
		


Credits:
--------

Thanks to Gene Apperson and Keith Vogel from Digilent for their contributions of HTTP server code and to Digilent in 
general for providing great microcontroller platforms at an accessible price.
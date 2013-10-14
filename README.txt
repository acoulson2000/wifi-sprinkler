WiH2O - ('wi-tü-o) Opensource WiFi-enabled Lawn Sprinkler Controller

Introduction:
-------------
This project is a result of my personal effort to create a WiFi-enabled lawn sprinkler controller for my home.

Currently, the chipKIT µc32 with a chipKIT WiFi shield are required, along with an SD card plugged into the slot on the WiFi
shield, which holds the web pages and the persisted schedule (in order to survive power outages).

The chipKIT µc32 is an Arduino-compatible microcontroller board based on the PIC32 processor made by Digilent. I went with the µc32 because I needed more RAM to hold the schedules. 
The chipKIT µc32 is also quite a bit faster, which is probably a good thing - even with the faster speed, serving up the
web pages is not exactly lightning fast. I suspect a MAX32 would also do the job - and would provide an opportunity to persist
the schedule in EPROM instead of on the SD card. Other WiFi shields might also be compatible.

The software consists of a sketch and related modules developed in the chipKIT MPIDE (available for download on Digilent's
website). This is a version of the Arduino IDE which is compatible with the chipKIT product line. A significant portion of
the project is code which implements a very basic webserver. Implementing an HTTP server from the ground up in c probably would
have been beyond my capability; fortunately, Digilent's Gene Apperson has written a basic implementation which I was able
to use as a starting point. Since starting the project, I've been in touch with Gene, and subsequently Keith Vogel, at 
Digilent. Keith has put together a somewhat more robust version of the HTTP server which would really be better to use...
I just haven't had time to go back an re-implement on that foundation. 

One of two varieties of Real Time Clock can be used (see App.h settings, below). You can either use RTC that is embedded in the
chipKIT PIC32's, or a DS1302-based daughter board connected via SCLK, IO, and CE pins.

You are free to use this project under the GNU LGPL license.


Requirements:
-------------

	** TODO **

	MPIDE 0715 or newer  	** This is important, as there were some WiFi and SD fixes since the prior official release, I believe.
	

	
Installation:
-------------


	
Configuration:
--------------
	
	** TODO - finish **

	In App.h, define these values:
		#define MAX_ZONES			6		// Number of spinkler zones (solenoid valves) in your yard

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
		
	If using the DS1302, set the interface pins in ClockWrapper.cpp:
		// Avoid lines used by WiFi shield or this server prog: 2,3,4,5,6,9,34,35,36 
		#define SCLK        39
		#define IO          40
		#define CE          41


Stuff left to do:
-----------------
Report last run time	     -	Show the last date/time each event ran
Configurable WiFi settings   -	Make the WiFi SSID and password configurable via USB.
Configurable # of events     -	Make the number of events available a user-configurable setting.
Start/End time validation    -	Check that none of the start time/run times overlap.

ChipKIT WF32		     -	Try on Digilents recently release (as of this writing) WF32 board, which incorporates
				the three components in this solution - WiFi and SD - in one board (still need an RTC).

Add weather intelligence     -	Add some weather awareness whereby the server process checks a weather API such
				as that provided by NOAA and adjusts the watering schedule accordingly.
				Options could include the ability to skip zone events or adjust the length and/or
				frequency based on weather conditions.

Port to Arduino Mega	     -	It would sure be nice to be able to run on a 'true' Arduino platform, such as 
				Arduino Mega. This presupposes that the Digilent Wifi shield can be made to work
				with Mega. I don't know if this is possible, but theoretically, I suppose you 
				could incorporate the Digilent DNETcK and DWIFIcK network and SD libraries into
				a generic Arduino project. Porting to Mega would also provide the ability to 	
				persist the schedule in EPROM, rather than writing it to the SD card.

Port to true Arduino platform - The idea here would be to port to all 'true' Arduino hardware; e.g, Mega + WiFly.
				I would expect that this would require a major rewrite unless an MRF24 shield is 
				available.


Credits:
--------

Thanks to Gene Apperson and Keith Vogel from Digilent for their contributions of HTTP server code and to Digilent in 
general for providing great microcontroller platforms at an accessible price.

Thanks to Dennis Clark at Servo Magazine for their article describing how to use Gene's original HttpServer 
implementation. They also host a zimp file of the HttpServer project in their downloads section (http://www.servomagazine.com/index.php/magazine/article/november2012_MrRoboto)

This project uses the Jquery, JQueryUI and JsRender templating librarys to render the console page. They are included under the MIT License:
	http://jquery.com/
	http://jqueryui.com/
	https://github.com/BorisMoore/jsrender

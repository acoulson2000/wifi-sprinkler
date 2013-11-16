WiH2O - ('wi-tü-o) Opensource WiFi-enabled Lawn Sprinkler Controller

Introduction:
-------------
This project is a result of my personal effort to create a WiFi-enabled lawn sprinkler controller for my home.

Currently, the chipKIT µc32 with a chipKIT WiFi shield are required, along with an SD card plugged into the slot on the WiFi
shield, which holds the web pages and the persisted schedule (in order to survive power outages).

The chipKIT µc32 is an Arduino-compatible microcontroller board based on the PIC32 processor made by Digilent. I went with the µc32
because I needed more RAM to hold the schedules. 
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
chipKIT PIC32's, or a DS1302-based daughter board connected via SCLK, IO, and CE pins. FYI: I used a very inexpensive DS1302 board
from China (originaly obtained here: http://www.dx.com/p/150179). It was necessary to add a pullup resistor to the I/O line 
for it to work correctly. See my blog here: http://wp.me/p14XJj-6d (austinlightuy.wordpress.com)

You are free to use this project under the GNU LGPL license.


Requirements:
-------------
    Hardware
    --------

    Micro SD Card               - This will hold the html and related files and is where your schedule will be stored so that
                                  it is persisted between reboots. The smallest you have will probably be plenty - the file
                                  sizes total less than 1MB total.
    
	Either:
    Digilent chipKIT uC32        - PIC32 Arduino-compatible microcontroller board made by Digilent
    Digilent chipKIT WiFi shield - WiFi shield with on-board SD card
    Micro-SD card                - A micro-SD card to hold the web page components - probably the smallest available is more 
                                   than enough
	or:
	Digilent chipKIT WF32        - A newer board that combines the functionality of the uC32 and WiFi shield into one board.
	
    Either:
     a DS1302 daughter board 
    or:
	 a 32.768Khz crystal      - The controller currently requires that you connect either a DS1302-based Real Time Clock 
                                   daughter board or that you enable the uC32's on-board RTC by soldering on a 32.768Khz crystal
                                  (see my blog for details: http://wp.me/p14XJj-6d).

    Some sort of board for controlling your sprinkler relays. In my blog entry, I describe how I used a second-hand triac 
    controller board which I drive directly from the uC32 IO pins (26 trhough 31, and 32).


    Software
    --------

    MPIDE 0715 or newer      ** This is important, as there were some WiFi and SD fixes since the prior official release, I believe.
	**NOTE**                 ** To use the WF32 instead of uC32 + WiFi shield, I had to obtain an even newer build of the supporting
                                libraries. Specifically, the pin numbers for the SPI support for the SD card drivers are different
								and require more recent library files. I obtained the latest updates from the source repository
								here: https://github.com/chipKIT32/chipkit-core
    

    
Installation:
-------------

Obtain the controller code and supporting webpage files from the code repository currently here: https://code.google.com/p/wifi-sprinkler

Copy the html components from the webpages directory to the root of the SD card.
Use Digilent's MPIDE to load the sketch, HttpServer.pde and related libraries. Edit configuration settings as described
in Configuration section below. 

If you choose to use a DS1302 RTC, install the chipKITDS1302 library (under the libraries folder) as described 
here: http://chipkit.net/started/learn-basics/party-libraries/		This library will likely be replaced with
a more common community version soon, although it will still have to be installed manually.

    
Configuration:
--------------
    
    HttpServer.pde:
        #define USE_OPEN_SECURITY       0       // Only has meaning if doing WiFi, 0 -> WPA2, 1 -> Open


                            
	int	requireAuth =	1;		// Set to 1 if you want webserver to require credentials, otherwise 0.
	
char	authCreds[] =	"yourusername:yourpassword";	// If you do, also define the authCreds below in the form "username:password"

        // Set to 1 if using DHCP, 0 if using a static IP       
        #define USE_DHCP    0                   // 0 -> Static IP, 1 -> DHCP

        ipServer = { 192,168,1,100 };           // If using static IP, specify IP address and port
        ptServer = 80;

        char * szSSID       = "yournetwork";    // the name of the network to connect to
        
        char * szPassPhrase    = "passphrase";  // pass phrase to use with WPA2: has no meaning if using open security



    In App.h, define these values:
        #define MAX_ZONES           6       // Number of spinkler zones (solenoid valves) in your yard

        #define MAX_EVENTS          15      // Number of events to support - must have corresponding 
                                            // initializations lines for zones array in App.cpp
                                            
        #define FIRST_VALVE_PIN     26      // Defines the I/O pin of the first sprinkler valve solenoid - this pin,
                                            // followed by the next 5 will control your sprinkler valves
                                            
        #define MASTER_VALVE_PIN    32      // If you have a master/backflow valve, this pin  will be used to control it.

        #define INVERT_PINS         'Y'     // If you want Arduino pin to be HIGH for Off, set to 'Y', normally set to 'N'
                                            // (my triac board uses reverse levels)
        #define INVERT_MASTER_PIN   'Y'     // Like INVERT_PINS, but for the master valve

        // To specify which Real Time Clock you will use, specify one of either:
        #define   RTCC_DS1302
        //    or:
        #define   RTCC_PIC32
        
    If using the DS1302, set the interface pins in ClockWrapper.cpp:
        // Avoid lines used by WiFi shield or this server prog: 2,3,4,5,6,9,34,35,36 
        #define SCLK        39
        #define IO          40
        #define CE          41


    Compile and upload to uC32, then open the serial monitor with a baud rate of 115200. 
    If all is well, you should see status updates showing the app starting up and connecting to your WiFi:

        Init DS1302                                                           -- or "Init PIC32", if you're using the embedded RTC
        10/18/13 08:55:16
        SD card initialized, CS pin 4
        Load config from SD
        WiFi attempt to connect to {yourssid}
        WiFi connection created, ConID = 1
        Please wait for up to 40 seconds for the connection to be established.
        WiFi connection established
        Initializing network stack
        Stack initialized
        IP Address: {your IP}
        Started listening
        Listening on port {your port}


    Launch your browser and go to: http://{your IP address}:{your port}  		(default, 192.168.1.100)
    


Stuff left to do:
-----------------
Sync Time with time server   -  Periodically sync RTC with time server..or at least browser time..RTC's seem to drift a lot.

Remove custom DS1302 lib     -  Originally, the DS1302 did not seem to work with all the libraries I tried. I wound
                                up creating a custom one () that seemed to work better, but I also added a pullup 
                                resistor to the I/O line. It's possible that was the problem all along - need to
                                try one of the existing libraries out there, such as https://github.com/msparks/arduino-ds1302
								or http://www.henningkarlsen.com/electronics/library.php?id=5
								or http://playground.arduino.cc/Main/DS1302
                
Configurable WiFi settings   -  Make the WiFi SSID and password configurable via USB.

Configurable # of events     -  Make the number of events available a user-configurable setting.

Start/End time validation    -  Check that none of the start time/run times overlap.

WiFi configuration           -  It would be REALLY nice to have the code start out in ad-hoc WiFi mode and 
                                provide a GUI for configuring it from there, including specifying the SSID of the WiFi network to 
                                join for 'normal' operation. Once configured, the system would restart and join the localnetwork.
                                Of course, some 'back door' would have to be provided in case there is a connection issue. Also,
                                providing access to the config settings from the main page would be desirable.

Migrate to new Digilent code  - Subsequent to starting this project using an early version of Http server project they
                                had provided to the community, I had a dialog with a couple of the engineers at Digilent
                                about a couple of issues. They have subsequently provided a cleaner and more robust
                                example implementation. This app should really be migrated to that newer starting 
                                platform, as it addresses several potential issues in the original implementation,
                                including support for multiple concurrent requests. 

Add weather intelligence     -  Add some weather awareness whereby the server process checks a weather API such
                                as that provided by NOAA and adjusts the watering schedule accordingly.
                                Options could include the ability to skip zone events or adjust the length and/or
                                frequency based on weather conditions.

Port to Arduino Mega         -  It would sure be nice to be able to run on a 'true' Arduino platform, such as 
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
implementation. They also host a zip file of the HttpServer project in their downloads section (http://www.servomagazine.com/index.php/magazine/article/november2012_MrRoboto)

This project uses the Jquery, JQueryUI and JsRender templating librarys to render the console page. They are included under the MIT License:
    http://jquery.com/
    http://jqueryui.com/
    https://github.com/BorisMoore/jsrender

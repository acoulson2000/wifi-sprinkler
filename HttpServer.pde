/************************************************************************/
/*																		*/
/*	HttpServer.pde	--	Example HTTP Server Application					*/
/*																		*/
/************************************************************************/
/*	Author: 	Gene Apperson											*/
/*  Copyright 2012, Digilent Inc., All Rights Reserved					*/
/************************************************************************/
/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License (GNU LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  To obtain a copy of the GNU LGPL, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/************************************************************************/
/*  Module Description: 												*/
/*																		*/
/*	This is the main module for an example HTTP server that runs on		*/
/*	Digilent chipKIT and Cerebot boards. It uses the Digilent DNETcK	*/
/*	and DWIFIcK libraries for network services.							*/
/*																		*/
/*	Hardware setup:														*/
/*		chipKIT Uno32													*/
/*			chipKIT WiFi Shield											*/
/*			chipKIT Basic I/O Shield									*/
/*		chipKIT uC32													*/
/*			chipKIT WiFi Shield											*/
/*			chipKIT Basic I/O Shield									*/
/*		chipKIT Max32													*/
/*			chipKIT WiFi Shield											*/
/*			chipKIT Basic I/O Shield									*/
/*		chipKIT Max32													*/
/*			Network Shield (no SD card file system available)			*/
/*			chipKIT Basic I/O Shield									*/
/*		Cerebot MX3cK													*/
/*			PmodBTN or PmodSWT - JA, lower row of pins					*/
/*			PmodBTN or PmodSWT - JB, lower row of pins					*/
/*			Pmod8LD - JD												*/
/*			PmodSD - JC													*/
/*			PmodNIC100 or PmodWiFi - JE									*/
/*		Cerebot MX4cK													*/
/*			PmodBTN or PmodSWT - JC, lower row of pins					*/
/*			PmodBTN or PmodSWT - JD, lower row of pins					*/
/*			Pmod8LD - JA												*/
/*			PmodSD - JK													*/
/*			PmodNIC100 or PmodWiFi - JB									*/
/*			(ensure that JP3 is in the INT3 position)					*/
/*		Cerebot MX7cK using internal MAC								*/
/*			PmodBTN or PmodSWT - JA, lower row of pins					*/
/*			PmodBTN or PmodSWT - JB, lower row of pins					*/
/*			Pmod8LD - JD												*/
/*			PmodSD - JF													*/
/*																		*/
/************************************************************************/
/*  Revision History:													*/
/*																		*/
/* 05/27/2012(GeneA): Created											*/
/* 09/01/2013(Andy Coulson): Modified for WiH2O							*/
/*																		*/
/************************************************************************/

#include	<plib.h>
#include	<string.h>
/* ------------------------------------------------------------ */
/*					Network Library Includes					*/
/* ------------------------------------------------------------ */

// Select only one of the following to match your hardware
//#include    <PmodNIC100.h>			// PmodeNIC100 (ENC424J600) is used
//#include    <PmodNIC.h>				// PmodNIC (ENC28J60) is used
//#include    <NetworkShield.h>			// Using the Network Shield, can also can be
										//   used for the Cerebot MX7cK or 32MX7
										// Internal PIC32 MAC, SMSC LAN8720 PHY
#include    <WiFiShieldOrPmodWiFi_G.h>	// MRF24WB0MA WiFi transceiver 

// Must be included
#include <DNETcK.h>                 // DNETcK IP Library

/* Only include if doing WiFi
** This can't be done automatically on a #ifdef of PMODWIFI_H as
** MPIDE will still parse the include line not knowing that it is
** in a #ifdef that is false. However MPIDE does recognize 
** comments, so we can comment it out and MPIDE will not include
** the WiFi library if not doing WiFi.
*/
#include <DWIFIcK.h>                // Specify DNETcK over WiFi

/* ------------------------------------------------------------ */
/*				Library Includes								*/
/* ------------------------------------------------------------ */

#include	<SD.h>

/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include	"Parser.h"
#include	"Render.h"
#include	"HtmlSource.h"
#include	"BoardDefs.h"
#include	"App.h"
#include	"Base64.h"

#if defined(RTCC_DS1302)
 #include <chipKITDS1302.h>
#elif defined (RTCC_PIC32)
 #include <RTCC.h>
#endif

#include	"ClockWrapper.h"

/* ------------------------------------------------------------ */
/*				Configuring this sketch							*/
/* ------------------------------------------------------------ */

// Set to 1 if using open security, otherwise WPA2 is used
//***DLC
#define USE_OPEN_SECURITY   0       // Only has meaning if doing WiFi, 0 -> WPA2, 1 -> Open

// Set to 1 if using DHCP, 0 if using a static IP
#define USE_DHCP    0               // 0 -> Static IP, 1 -> DHCP

/* Define the IP address of the server, and the IP port number
** that it monitors for an inbound connection. The client programs
** need to know these values in order to attempt to make a connection
** to the server.
*/
#if (USE_DHCP == 0)
IPv4        ipServer = { 192,168,1,10 };
#endif // End No DHCP

/* Define the port on which the server will listen for inbound
** connection requests. The standard HTTP port number is 80.
*/
uint16_t    ptServer = 80;

/* Set to 1 if you want webserver to require credentials, otherwise 0.
** If you do, also define the authCreds below in the form "username:password".
*/
int			requireAuth =	1;
char		authCreds[] =	"yourusername:yourpassword";

// This will hold the encoded form
char encodedCreds[64];
	
/* ------------------------------------------------------------ */
/*				Local Type and Constant Definitions				*/
/* ------------------------------------------------------------ */

#define cchLineMax	512

/* State codes for the network server state machine in the StackTask
** function.
*/
typedef enum {
	stsWiFiConnect,
	stsWiFiConnecting,
    stsInitialize,
    stsInitializing,
    stsListen,
    stsIsListening,
    stsProcessClients,
	stsWiFiInitError,
    stsInitError,
    stsListenError,
    stsRestart,
} STS;

/* State codes for the state machine that turns the stream of text
** from the client into a series of lines of text.
*/
typedef enum {
    stcIdle,
    stcConnected,
    stcBeginLine,
    stcNextChar,
    stcGotCr,
    stcWaitForBuffer,
    stcCloseConnection,
	stcInputError
} STC;

/* State codes for the HTTP state machine that parses and processes the
** HTTP request messages.
*/
typedef enum {
	sthIdle,
	sthRequestStart,
	sthWaitHeaderLine,
	sthParseHeaderLine,
	sthBeginResponse,
	sthSendLocalResource,
	sthBeginFileTransfer,
	sthBeginFileBuffer,
	sthSendFileBuffer,
	sthEndFileTransfer,
	sthProcessGetRequest,
	sthGetInvalidResource,
	sthGetFileResource,
	sthGetLocalResource,
	sthProcessHeadRequest,
	sthProcessPostRequest,
	sthProcessPutRequest,
	sthProcessTraceRequest,
	sthProcessOptionsRequest,
	sthProcessDeleteeRequest,
	sthProcessInvalidRequest
} STH;

/* This symbol defines the number of milliseconds to wait for
** data to arrive after accepting a client. If this limit is
** exceeded, we consider the connection to have been lost.
*/
const uint32_t	msWaitLimit    = 5000;

const uint32_t	msListenLimit = 30000;
const uint32_t	msClientTimeout = 1000;

/* Use LEDs on the WiFiShield or PmodShield or uC32 or LD3 and LD4 
   on the WF32 for network status indicators */
#if defined(_BOARD_UNO32_) || defined(_BOARD_UNO_) || defined(_BOARD_UC32_) || defined(_BOARD_WF32_64_)
const int    pinLedConnect = 3;
const int    pinLedInitialized = 5;
#elif defined (_BOARD_WF32_)
const int    pinLedConnect = 47;
const int    pinLedInitialized = 48;
#endif	

#define	pinSdCs	PIN_SDCS

/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */

extern int	idresRequest;
extern int	rtypResource;
extern int	cbResource;
extern char	szResourcePath[];
extern int	errParse;
extern int	errHttpRequest;

/* Declare the server and client objects. This server only
** accetps a single client at a time, so only a single client
** object is needed. The client object represents the client
** side of the TCP connection.
*/
TcpServer   tcpServer;
TcpClient	tcpClient;

/* The server and client are implemented as state machines
** these variables hold the current state for the two
** state machines
*/
STS	stsCur;			//network server current state
STC	stcCur;			//newwork client current state
STH sthCur;			//HTTP server current state

/* This buffer holds HTTP lines of text received from the client
** for parsing.
*/
char	szLineBuffer[cchLineMax+1];
int		cchLineCur;
char *	pchLineCur;
int		fInputReady;
int		fRequestStart;

File	fhData;			// data file object for reading files an SD card

/* ------------------------------------------------------------ */
/*				Local Variables									*/
/* ------------------------------------------------------------ */

const uint32_t	stsBlinkMs = 3000;
const uint32_t	stsBlinkMsDurationMs = 50;
uint32_t    stsCurMs;
bool        stsConnLedCur;
uint32_t    msBegin;
uint32_t    msConnect;			//system time at which connected
uint32_t	msClientActivity;	//system time of last client activity
DNETcK::STATUS status;

/* These are used when the resource requested by the HTTP client
** is contained in a file on the SD card file system.
*/
extern uint8_t	rgbFileBuf[];

/* ------------------------------------------------------------ */
/*                WiFi Variables                                */
/* ------------------------------------------------------------ */

#if defined(USING_WIFI)

//***DLC
char * szSSID       = "yourssid";			// the name of the network to connect to
char * szPassPhrase	= "yourpassphrase";			// pass phrase to use with WPA2
											//   has no meaning if using open security
int	conID			= DWIFIcK::INVALID_CONNECTION_ID; 
     
const uint32_t msWiFiWaitLimit  = 40000;	// it could take awhile for the AP to respond,
											// especially if we have to calculate a PSK key
uint32_t    msWiFiBegin;
int         fRestartWiFi = false;

#endif  // end WiFi

/* ------------------------------------------------------------ */
/*				Forward Declarations							*/
/* ------------------------------------------------------------ */

void StackInit();
void AppInit();
void AppTask();
void ServerTask();
void ClientTask();
void HttpTask();
void InitHttpRequest();
void PutResponseHeader();
void PutResponseBody();

/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */
/***    setup
**
**  Parameters:
**      none
**  
**  Return Value:
**      none
**  
**  Errors:
**      none
**  
**  Description:
**      Program entry point as defined by the chipKIT/Arduino
**      system. This function performs one-time initialization
**      required by the sketch.
**      In this particular case, it initializes the network stack
**      and begins listening for an inbound connection.
**      It also initializes the serial port to be used by the
**      serial monitor window to present status about the server
**      operation.
*/

void setup() {

    /* We're going to use printing to the serial monitor window as a
    ** surrogate for logging the data. Initialize the serial port
    ** for our output, and print a banner message to indicate that
    ** we are up and running.
    */
    Serial.begin(115200);
    Serial.println("Digilent Example HTTP Server");
    Serial.println("");
	
    /* Initialize everything. */
	ClockInit();
    AppInit();
    StackInit();

	base64_encode((char *)encodedCreds, (char *)authCreds, strlen(authCreds));
}

/* ------------------------------------------------------------ */
/***    loop
**
**  Parameters:
**      none
**
**  Return Value:
**      none
**
**  Errors:
**      none
**
**  Description:
**      This function is defined by the chipKIT/Arduino system.
**      It is called repeatedly, and implements the event loop
**      behavior of the sketch.
*/

void loop()
    {

    /* Run the application.
    */
    AppTask();

    /* Run the network server state machines.
    */
    ServerTask();
	ClientTask();
	HttpTask();

}

/* ------------------------------------------------------------ */
/***    StackInit
**
**  Parameters:
**      none
**
**  Return Value:
**      none
**
**  Errors:
**      none
**
**  Description:
**      Perform one-time startup initialization of the network
**      server.
*/

void StackInit() {

    /* Set up the pin used for the connection status output.
    */
    pinMode(pinLedConnect, OUTPUT);		//connection status LED
    pinMode(pinLedInitialized, OUTPUT);		//connection status LED
    digitalWrite(pinLedConnect, LOW);
    digitalWrite(pinLedInitialized, LOW);

	/* Indicate that we are waiting for the start of the next
	** HTTP Request message.
	*/
	stcCur = stcIdle;
	sthCur = sthIdle;

	/* Mark the input line buffer as being empty.
	*/
	fInputReady = false;

    /* Start the server state machine in the stsInitialize state
    ** to begin the process of initializing the network stack. If
	** we're using a WiFi connection, we need to connect to the
	** WiFi network first, so in that case, start in the stsWiFiConnect
	** state.
    */
#if defined(USING_WIFI)
	// WiFi
	stsCur = stsWiFiConnect;
	fRestartWiFi = false;
#else
	// Not WiFi
	stsCur = stsInitialize;
#endif  // End USING_WIFI

    /* We do not want to spend time waiting for the network code
    ** to return, so always specify an immediate return to all of
	** the methods
    */
    DNETcK::setDefaultBlockTime(DNETcK::msImmediate);  
}

/* ------------------------------------------------------------ */
/***    ServerTask
**
**  Parameters:
**      none
**
**  Return Value:
**      none
**
**  Errors:
**      none
**
**  Description:
**		This function manages the network stack and performs the
**		network server related behavior. It establishes the connection
**		to the network and listens for and accepts inbound client
**		connection requests.
**		The network server behavior is implemented as a state machine.
**		No path through this function should take more than a small
**		number of milliseconds to execute.
*/

void ServerTask()
    {
	IPv4    myIP;
    
    switch(stsCur) {
        /* ------------------------------------------------------------ */
        /*                WiFi Intialization                            */
        /* ------------------------------------------------------------ */
#if defined(USING_WIFI)
        case stsWiFiConnect:
            /* Attempt to connect to the WiFi network.
            */
            Serial.print("WiFi attempt to connect to "); Serial.println(szSSID);
    #if (USE_OPEN_SECURITY == 1)
            if ((conID = DWIFIcK::connect(szSSID, &status)) != DWIFIcK::INVALID_CONNECTION_ID) {                   // Open
    #else
            if ((conID = DWIFIcK::connect(szSSID, szPassPhrase, &status)) != DWIFIcK::INVALID_CONNECTION_ID) {     // WPA2
    #endif  // End USE_OPEN_SECURITY
                Serial.print("WiFi connection created, ConID = ");
                Serial.println(conID, DEC);

                Serial.print("Please wait for up to ");
                Serial.print(msWiFiWaitLimit/1000, DEC);
                Serial.println(" seconds for the connection to be established.");

                msWiFiBegin = millis();
                stsCur = stsWiFiConnecting;
            }
            else {
                Serial.print("Unable to make a WiFi connection, status: ");
                Serial.println(status, DEC);
                stsCur = stsWiFiInitError;
            }
            break;

        case stsWiFiConnecting:
            /* Wait to complete the connection to the WiFi network.
			*/
            if (DWIFIcK::isConnected(conID, &status)) {
                Serial.println("WiFi connection established");
                stsCur = stsInitialize;
            } else 
			if (DNETcK::isStatusAnError(status)) {
                Serial.print("WiFi connection failed to establish, status: ");
                Serial.println(status, DEC);
                stsCur = stsWiFiInitError;
            }  else if ((millis() - msWiFiBegin) > msWiFiWaitLimit) {
				Serial.println("WiFi connection timeout");
				stsCur = stsWiFiInitError;
            }

			if (millis() > (stsCurMs + stsBlinkMs)) {
				if (!stsConnLedCur) {
					digitalWrite(pinLedConnect, HIGH);
					stsConnLedCur = true;
				}
			}

			if (millis() > (stsCurMs + stsBlinkMs + stsBlinkMsDurationMs)) {
				if (stsConnLedCur) {
					digitalWrite(pinLedConnect, LOW);
					stsConnLedCur = false;
					stsCurMs += stsBlinkMs;
				}
			} 

            break;
#endif // USING_WIFI

		/* ------------------------------------------------------------ */
		/*                IP Network Intialization                      */
		/* ------------------------------------------------------------ */

        case stsInitialize:
            /* Initialize the network stack and establish the server IP
            ** address. Since this example works entirely on the local network
            ** and doesn't need access off of the local network, there is
            ** no need to specify a gateway for routing.
            */
            Serial.println("Initializing network stack");
#if (USE_DHCP == 1)
            DNETcK::begin();			// use DHCP to get IP address
#else
            DNETcK::begin(ipServer);	//use hard IP address
#endif  // USE_DHCP
            msBegin = millis();			// remember when we started this operation
			stcCur = stcIdle;
            stsCur = stsInitializing;
			digitalWrite(pinLedConnect, HIGH);
			break;

        case stsInitializing:
            /* Wait for the stack to initialize before going to the
            ** idle state.
            */
            if (DNETcK::isInitialized(&status)) {
				/* Initialized successfully, start listening for a
				** connection.
				*/
				digitalWrite(pinLedInitialized, HIGH);
                stsCur = stsListen;
                Serial.println("Stack initialized");

                /* Print our IP address
				*/
                if (DNETcK::getMyIP(&myIP)) {
                    Serial.print("IP Address: ");
                    Serial.print(myIP.rgbIP[0], DEC);
                    Serial.print(".");
                    Serial.print(myIP.rgbIP[1], DEC);
                    Serial.print(".");
                    Serial.print(myIP.rgbIP[2], DEC);
                    Serial.print(".");
                    Serial.println(myIP.rgbIP[3], DEC);
                }
                else {
                    Serial.println("Unable to determine IP");
					stsCur = stsInitError;
                }
            }
            else if(DNETcK::isStatusAnError(status)) {
				/* Got a hard error trying to initialize.
				*/
                stsCur = stsInitError;
            }
            else if ((millis() - msBegin) > msWaitLimit) {
				/* Timed out trying to initialize.
				*/
                stsCur = stsInitError;
            }
            break;

        case stsListen:
            /* Start listening for a new inbound connection request.
            */
            if (tcpServer.startListening(ptServer, &status)) {
                /* We are listening for a new connection.
                */
                Serial.println("Started listening");
                stsCur = stsIsListening;
            }
            else {
                /* Failed for some reason. Restart everything.
                */
                stsCur = stsListenError;
            }
            break;

        case stsIsListening:
            /* We are now waiting for a new connection to come in.
            ** This state is redundant in this particular application.
            ** It is being used to print on the serial monitor to
            ** show that we passed through this state and are actively
            ** waiting for a connection.
            */
            if (tcpServer.isListening()) {
                /* We've successfully entered the listening state.
                */
                Serial.print("Listening on port ");
                Serial.println(ptServer,DEC);
                stsCur = stsProcessClients;
            }
            else {
                /* Failed to begin listening for a client.
                */
                stsCur = stsListenError;
            }
            break;

        case stsProcessClients:
            /* Check if there is a client asking for a connection.
            */
            if (tcpServer.availableClients() > 0) {
                /* There is at least one client waiting. See if we are
                ** able to accept it.
                */
                if (stcCur == stcIdle) {
                    /* We are not currently connected, so accept this client.
                    */
                    tcpServer.acceptClient(&tcpClient);
                    stcCur = stcConnected;
                    msConnect = millis();
					Serial.println();
                    Serial.println("Accepted client");
                }
            }

            /* If available clients is zero and we are not listening, 
			** something is wrong! The only time we might not be listening is
            ** if we have no more server sockets available, but then
            ** we must have available clients.
            */
            else if (!tcpServer.isListening()) {
                stsCur = stsListenError;
            }
            break;

        /* ------------------------------------------------------------ */
        /*                Catastrophic errors                           */
        /* ------------------------------------------------------------ */

#if defined(USING_WIFI)
        case stsWiFiInitError:
            Serial.print("Failed to connect to ");
            Serial.print(szSSID);
            Serial.println(" WiFi network.");
            stsCur = stsRestart;
            break;
#endif  // End WiFi
      
        case stsInitError:
            /* Failed to initialize the stack. "Log" the event by writing
            ** a message to the serial monitor window. Since we can't do
            ** anything without a functioning stack, start the initialization
            ** process over.
            */
            Serial.println("Failed to initialize network stack!");
            stsCur = stsRestart;
            break;

       case stsListenError:
            /* Failed to start listening. "Log" the event by writing
            ** a message to the serial monitor window. Since we can't do
            ** anything without a functioning stack, start the initialization
            ** process over.
            */
            Serial.println("Failed to start listening!");
            stsCur = stsRestart;
            break;

       case stsRestart:
            Serial.println("Restarting Network");

            /* It is always safe to close the socket, even if it was never opened
            ** this will ensure cleanup of the sockets.
            */
            tcpServer.close();
			tcpClient.close();
			stcCur = stcIdle;

            // completely shutdown the IP stack.
            DNETcK::end();

#if defined(USING_WIFI)
            /* Only reconnect to the AP if we lost the connection.
            ** While it might seem reasonable to do this on any catastrophic
            ** failure, hitting the AP just because the server went down (stcNoConnection)
            ** could seriously overload the AP as everyone would be getting this error
            ** and they would all be reconnecting to the AP concurrently.
            ** also we want to make sure we don't shutdown the connection while
            ** the WiFi is attempting to reconnect; the WiFi MAL does not take kindly to that.
            */
            if((!DWIFIcK::isConnected(conID, &status) && DNETcK::isStatusAnError(status)) ||
						fRestartWiFi) {
                Serial.println("Restarting WiFi");
                fRestartWiFi = false;
				digitalWrite(pinLedConnect, LOW);
                DWIFIcK::disconnect(conID);
                conID = DWIFIcK::INVALID_CONNECTION_ID;
                stsCur = stsWiFiConnect;
            }

            /* If we are still connected to the AP,
            ** Just attempt to restart the network stack
            */
            else {
                stsCur = stsInitialize;
            }
#else
			/* If not using WiFi, just restart the network stack.
			*/
            stsCur = stsInitialize;
#endif   // USING_WIFI
            break;

        default:
            /* We're in an invalid state. Treat this as an initialization
            ** errror and start over.
            */
            Serial.println("Internal error: invalid state!");
            stsCur = stsRestart;
            break;

    }

    /* Since there is no real-time kernel running to schedule
    ** concurrent tasks, the network stack uses a polling model.
    ** It is necessary to call the stack polling function
    ** regularly to keep the stack alive.
    */
    DNETcK::periodicTasks();

}

/* ------------------------------------------------------------ */
/***	ClientTask
**
**	Parameters:
**
**	Return Value:
**
**	Errors:
**
**	Description:
**		This function is a state machine that handles the input stream
**		from the client connection. The HTTP protoocol is a line
**		based text protocol. This state machine reads lines from the
**		input stream and places them into a line buffer for parsing
**		and interpretation by the HttpTask code.
**		This function is implemented as a state machine. No path through
**		this function should take more than a small number of milliseconds
**		to execute.
*/

void ClientTask()
	{
	char	ch;

	switch(stcCur) {
		case stcIdle:
			/* Not currently connected, nothing to do.
			*/
			fInputReady = false;
			break;

		case stcConnected:
			/* We have just accepted a client connection, perform any
			** one-time action needed when a new connection has been
			** accepted.
			*/
			Serial.println("Waiting for request");
			digitalWrite(pinLedConnect, HIGH);
			msClientActivity = millis();
			stcCur = stcBeginLine;
			break;

		case stcBeginLine:
			/* Wait for the beginning of the next line in a
			** request message.
			*/
			cchLineCur = 0;
			pchLineCur = &szLineBuffer[0];

			if (tcpClient.available() > 0) {
				/* Something has shown up in the input buffer, so go
				** on to the state to read the next line into the
				** line buffer.
				*/
				stcCur = stcNextChar;
			}
			else if (!tcpClient.isConnected()) {
				/* The connection has been lost.
				*/				
				stcCur = stcCloseConnection;
			}
			else if ((millis() - msClientActivity) > msClientTimeout) {
				/* Timed out for lack of client activity.
				*/
				stcCur = stcCloseConnection;
			}
			break;

		case stcNextChar:
			/* Read the next character from the input stream
			** and place in the line buffer. The line ends
			** when we see a CR LF. If the character read is
			** a CR, leave this state.
			*/
			if (tcpClient.available() > 0) {
				msClientActivity = millis();
				tcpClient.readStream((byte *)&ch, 1);
				if (ch != 0x0D) {
					if (cchLineCur < cchLineMax) {
						*pchLineCur = ch;
						pchLineCur += 1;
						cchLineCur += 1;
					}
				}
				else {
					stcCur = stcGotCr;
				}
			}
			else if (!tcpClient.isConnected()) {
				/* The connection has been lost.
				*/				
				stcCur = stcCloseConnection;
			}
			else if ((millis() - msClientActivity) > msClientTimeout) {
				/* Timed out for lack of client activity.
				*/
				stcCur = stcCloseConnection;
			}
			break;

		case stcGotCr:
			/* We came to the CR that is the beginning of the
			** line end sequence. Terminate the input string.
			*/
			*pchLineCur = '\0';
			if (tcpClient.available() > 0) {
				msClientActivity = millis();
				tcpClient.readStream((byte *)&ch, 1);
				if (ch == 0x0A) {
					/* We've seen the CR LF line end sequence.
					** Set the flag to indicate that there is a
					** line to be processed in the input buffer,
					** and then wait for it to be processed.
					*/
					stcCur = stcWaitForBuffer;
					fInputReady = true;
				}
				else {
					/* We got a CR not followed by a LF.
					** the HTTP request messge is messeed up.
					** This would be a good point to do some
					** kind of error recovery.
					*/
					stcCur = stcInputError;
				}
			}
			else if (!tcpClient.isConnected()) {
				/* The connection has been lost.
				*/				
				stcCur = stcCloseConnection;
			}
			else if ((millis() - msClientActivity) > msClientTimeout) {
				/* Timed out for lack of client activity.
				*/
				stcCur = stcCloseConnection;
			}
			break;

		case stcWaitForBuffer:
			/* Wait for the line in the buffer to be processed
			** so that we can start reading the next line.
			*/
			if (!fInputReady) {
				stcCur = stcBeginLine;
			}
			break;

		case stcCloseConnection:
			/* Close the connection to the client and go back to
			** the idle state.
			*/
			Serial.println("Closed Connection");
			tcpClient.close();
			stcCur = stcIdle;
			break;

		case stcInputError:
			Serial.println("!Input error");
			tcpClient.close();
			stcCur = stcIdle;
			break;

		default:
			break;
	}

    /* Since there is no real-time kernel running to schedule
    ** concurrent tasks, the network stack uses a polling model.
    ** It is necessary to call the stack polling function
    ** regularly to keep the stack alive.
    */
    DNETcK::periodicTasks();

}

/* ------------------------------------------------------------ */
/***	HttpTask
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		This function is a state machine that handles HTTP Request
**		messages and generates the HTTP responses.
*/

void
HttpTask()
	{
	static int	cbCur;		//these must be static so that their contents are
	static int	ibCur;		// preserved across multiple executions of this function
	int			cb;

	switch(sthCur) {
		case sthIdle:
			/* Wait for the start line of an HTTP Request message to come in.
			*/
			if (fInputReady) {
				/* There is a line ready to process. This would be the
				** start line of the HTTP request.
				*/
				sthCur = sthRequestStart;
			}
			break;

		case sthRequestStart:
			/* The line buffer contains the start line of the HTTP
			** request message.
			*/
			Serial.println(szLineBuffer);
			InitLineParse(szLineBuffer);
			InitHttpRequest();
			ParseHttpRequest();
			sthCur = sthWaitHeaderLine;
			fInputReady = false;		//indicate the line has been processed
			break;

		case sthWaitHeaderLine:
			/* Wait for the next header line in the HTTP requeste message
			** to be received.
			*/
			if (fInputReady) {
				sthCur = sthParseHeaderLine;
			}
			break;

		case sthParseHeaderLine:
			/* Process the header line in the input buffer.
			*/
			if (cchLineCur != 0) {
				//Serial.println(szLineBuffer);
				InitLineParse(szLineBuffer);
				ParseHttpHeader();
				sthCur = sthWaitHeaderLine;
			}
			else {
				/* A blank line indicates the end of the header lines and the
				** beginning of the message body.
				*/
				sthCur = sthBeginResponse;
			}
			fInputReady = false;		//indicate the line has been processed
			break;

		case sthBeginResponse:
			/* We got an empty line. This is the end of the headers.
			** Since we are only processing HTTP GET requests, there
			** won't be a message body. In response to a GET request
			** we need to send the requested resource.
			*/
			errHttpRequest = errParse;
			// IF authorization is required, first check whether there was a
			// valid authorizatio header found during parse. If not, set flag
			// to send 401 and advance to next state. 
			if (requireAuth == 1 && errHttpRequest == errNotAuthorized) {
				idresRequest = idres401;
				sthCur = sthSendLocalResource;
// SHOULD WE END EARLY VIA THIS CODE?
/*				PutResponseStart();
				PutResponseHeader();
				stcCur = stcCloseConnection;
				sthCur = sthIdle;
				msClientActivity = millis();
*/
				break;
			}
			if (errHttpRequest == errOK) {
				if (idresRequest == idresFile) {
					sthCur = sthBeginFileTransfer;
				}
				else {
					sthCur = sthSendLocalResource;
				}
			}
			else if (errHttpRequest == errNotModified) {
				PutResponseStart();
				PutResponseHeader();
				stcCur = stcCloseConnection;
				sthCur = sthIdle;
				msClientActivity = millis();
				break;
			}
			else if (errHttpRequest == errNotFound) {
				idresRequest = idres404;
				sthCur = sthSendLocalResource;
				break;				
			}
			else {
				sthCur = sthIdle;
			}
			break;

		case sthSendLocalResource:
			/* Generate the HTML that makes up the body of the
			** response message.
			*/
			PutResponseStart();
			PutResponseHeader();
			PutResponseBody();

			/* We don't know the content size of the hard coded pages
			** that we are rendering 'on the fly', so we can't send a
			** content-size header line. For these pages, we close the
			** connection so that the browser knows it is the end
			** of the stream. For resources that come out of a file
			** we know the size, so we don't need to close the connection
			** for the client to know when all of the data for the
			** resource has been sent.
			*/
			stcCur = stcCloseConnection;
			sthCur = sthIdle;
			msClientActivity = millis();
			break;

		case sthBeginFileTransfer:
			/* The requested resource is in a file. Transferring a file
			** can potentially take a long time, so the process of sending
			** a file is broken up into several states so that we never
			** block for an extended period of time.
			*/
			fhData = SD.open(szResourcePath, FILE_READ);
			if (fhData) {
				cbResource = fhData.size();
			}
			else {
				errHttpRequest = errNotFound;
			}

			/* Send the start line for the response.
			*/
			PutResponseStart();
			if (errHttpRequest == errOK) {
				/* If we didn't get any errors, we can send the header lines
				** and then the file contents.
				*/
				PutResponseHeader();
				sthCur = sthBeginFileBuffer;
			}
			else {
				/* We got an error, so there is nothing else to send after the
				** response start line.
				*/
				sthCur = sthIdle;
			}
			break;

		case sthBeginFileBuffer:
			/* Read the next buffer of data from the file.
			*/
			msClientActivity = millis();
			if (fhData.available() > 0) {
				ibCur = 0;
				cbCur = ReadFileBuffer(rgbFileBuf, cbFileBufMax);
				sthCur = sthSendFileBuffer;
			}
			else {
				sthCur = sthEndFileTransfer;
			}
			break;

		case sthSendFileBuffer:
			/* Send the contents of the file buffer. Depending on network activity
			** and the size of the buffer, writeStream may not succeed in writing
			** the entire buffer in one call. We stay in this state until the
			** entire contents of the buffer has been written successfully.
			*/
			msClientActivity = millis();
			if (cbCur > 0) {
				cb = tcpClient.writeStream((byte *)&rgbFileBuf[ibCur], cbCur);
				ibCur += cb;
				cbCur -= cb;
			}
			else {
				sthCur = sthBeginFileBuffer;
			}
			break;

		case sthEndFileTransfer:
			/* The entire contents of the file has been sent, so close up and go
			** back to the idle state to wait for the next HTTP request message.
			*/
			msClientActivity = millis();
			fhData.close();
			sthCur = sthIdle;
			break;

	}

    /* Since there is no real-time kernel running to schedule
    ** concurrent tasks, the network stack uses a polling model.
    ** It is necessary to call the stack polling function
    ** regularly to keep the stack alive.
    */
    DNETcK::periodicTasks();

}

/* ------------------------------------------------------------ */
/***	ProcName
**
**	Parameters:
**
**	Return Value:
**
**	Errors:
**
**	Description:
**
*/

/* ------------------------------------------------------------ */

/************************************************************************/


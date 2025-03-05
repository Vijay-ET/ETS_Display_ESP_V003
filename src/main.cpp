// https://randomnerdtutorials.com/esp8266-web-server-spiffs-nodemcu/
// https://techtutorialsx.com/2017/12/17/esp32-arduino-http-server-getting-query-parameters/

/***********************************************************
		 ------------	Release Details	-----------
 ***********************************************************/


/*
--> V001. Date :- 13.09.24 
	+ Changed the Old touch display interface with the new LAN Web-interface and integrated its functionality.
		* The new LAN interface has only 3 pages currently Index, Control, Display.
		* Deleted the Webconfig completely for now.
	+ Through the Control page user can control the device.
		* Can change the Set values V,I,P,R .
		* Turn on the device.
		* Change the mode.
		* User can also see the status of the device like monitor values, which mode device is in etc.
	+ Through the Display page user can see the device status.
		* User can see the monitor values.
		* User can see what mode device is in.
		* Status of the device like Standby/Run etc.
	+ Index page is the default page that will be opened when user entered the IP address.
		* it contains navigation buttons to Control and Display pages.
	+ Made the UI/UX changes to increase user experience.
	+ Made changes in Wi-Fi module code to make sure connection between Web-interface and the device lasts longer.
	+ Updated the Web-interface set value limits as per the Device parameters.
	+ Added the functionality to show faults like Ovp, Ocp, Otp in both Control and Display pages.
	+ Added the functionality to send back the source as WLAN/Local to STM. So that WLAN will be shown on the Touch display when Web-Interface is connected.
	+ Added the functionality not to change the mode from Web-Interface when device is in Run state.(Added this to replicate the actual device. When device is in Run mode device is not allowing to change the mode.)
	+ Integrated the commands and callback functions in Wi-Fi Module for data transfer so as to replicate the STM and PIC communication.										

--> V002. Date :- 22.10.24
	+ Integrated all 3 HTML pages(Index, Control, Display) into a single HTML file.
	+ Increased the Frequency of Microcontroller from 80MHZ to 160MHZ.
	+ Webpage loading time is reduced.
		In the previous release version, the Web interface was taking 4-5 seconds to load.
		In this new release version that loading time has been reduced.
	+ Taken care of the Multiple connected clients(multiple Webpages).
		Previously if multiple clients are connected, Wi-Fi module was behaving Unexpectedly.
		This has been taken care. And added the functionality so that all connected webpages will be in sync.
	+ Updated Touch Display for Local Source (WLAN/FRONT).
	+ Infinite Resistance Showing bug in Monitor values is fixed.
	+ Implemented functionalities in Web-Interface for Better User Experience.
        Added Alert messages for Communication Errors & Synchronisation Errors.
        Implemented Automatic Reload Functionality if Something Unexpected Happens Between Server(Wi-Fi) & Client(Webpage).
        Added Alert messages When user want to change the mode when device is Running.(ON).
        Implemented functionality to display real-time changes in set values as the user continuously adjusts the slider.
        RUN/Standby and Local/WLAN Buttons were implemented accordingly as per the Existing LAN interface.
	+ Implemented the functionality to generate Wi-Fi access point based on the Device Serial number.  Ex :- ETS-LAB/HP-12.23.759
        By this change Wi-Fi Access point will be generated based on the Serial number of Each power supply. If in case Device doesn't have any Serial number configured it will generate default access point as 
        ETS-Display.
        Serial number is received using Kompid command from the STM and Stored in EEPROM to read it at the Wi-Fi init.
	+ Implemented the Watchdog Functionality for monitoring Unexpected behaviours.
	+ Increased the size of Serial RX buffer(300 bytes) to match with STM.
	+ Matched the Command and Callback functions Structure with STM.
	+ Wi-Fi module reset issue.
		The continuous data inflow from STM was causing buffer overflow (or) data overwriting, leading to repeated Wi-Fi module resets, and getting stuck in a reading loop. 
		This was resolved by clearing the Wi-Fi RX buffers at the start and adding a watchdog monitor.


--> V003. Date :- 
	+ Added 2 more pages in Web-Interface Configuration, Protection.
	+ Update Set values struct in javascript with user input values from web-interface everytime they are updated.
	+ Also added the previous one to new page value inputs also.(Maybe New pages parameter input giving.)
	+ Updated the Rmin limits to -160 milli amps
	+ Given Restrictions for user inputs. If given beyond the min & max limits of respective parameters.
	+ Given Titles to notify users how to turn of particular Fetaure. when mouse is taken near that input text box.
	+ Strictly converted the passed parameters to be numbers in javascript.
	+ Upgraded the Hand-Shake(Ping-Pong) messages b/w Wi-Fi moudle and Web-interface for Websocket commuication.
	+ Implemented Modal boxes in javascript to notify users about different errors in a Non-blocking way to make the background communication work.
	  Also added the functionality of notifying the user when STM is Freezed.
	+ Added the functionaity to restrict Vmpp & Impp values to be in the range of calculations from the web-interface itslef.
	+ Added Foldback & FoldbackTm Commands b/w Wi-Fi module & Web-Interface.
	+ Discarded the Ambiguties b/w STM & Wi-Fi module with data types of Foldback & FoldbackTm.

	Todo :-
	+ Add cache control mechanism in the browser end so that Front-end files will be fetched faster.
		Cache Control:

		You might want to include a Cache-Control header to optimize how browsers cache the file. For example, if the file doesn't change often, 
		you could tell browsers to cache it for a longer period.s
		Copy code
			AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/combined.html", "text/html");
			response->addHeader("Cache-Control", "max-age=3600");  // Cache for 1 hour
			request->send(response);
		This prevents the browser from re-fetching the file on every request, reducing load times and server stress.

*/


/***********************************************************
 * Defines
 ***********************************************************/

/***********************************************************
 * Includes
 ***********************************************************/
#include <EEPROM.h>
#include "main.h"
#include "d_display_com.h"
#include "a_display_com.h"
#include "d_timer.h"
#include "a_ws_com.h"
#include "a_graph.h"
#include "a_ms.h"
#include <stdlib.h>
#include <ArduinoOTA.h>
#include <Updater.h>
#include "Serial_prints.h"


// #define ELEGANT_OTA_ENABLE
#ifdef ELEGANT_OTA_ENABLE
#include <ElegantOTA.h>
#endif

/***********************************************************
 * Module variables
 ***********************************************************/
// Version of the ESP Firmware.
#define ESP_RELEASE_VERSION V003
#define APP_FILE_SIZE 0x64000
#define WEB_FILE_SIZE 0x7A000
#define CHUNK_SIZE 256

#define DEBUG_ESP_HTTP_SERVER
#define DEBUG_ESP_UPDATER

AsyncWebServer server(80);
com_drv_t com_drv;
Netzteil_Parameters_t NtParam;
uint8_t ms_available;
Timer_t lastNoClientMessage ;
Timer_t Ws_last_Ping_Time;
Timer_t Ws_Ping_Time;

uint8_t counter = 0;
// uint8_t prio_cnt = 0;
// uint8_t local_prio = 0;
Timer_t msstate_timer;
Timer_t measure_timer;
MasterSlave_t msconfig_cpy = NtParam.msconfig;
scriptmode_state_e scriptstate;
bool first_status = true;

// V: variables related to wifi
bool Wifi_ready = false;
bool Wifi_is_stored = false;
char *Wifi_string;    
// char *default_wifi_str = "ETS-India-Lab-2";	
char *default_wifi_str = "ETS-Vijay";				
const char *Wifi_password = "12345678";
char komp_wifi[40] = "ETS-";
uint8_t komp_wifi_len;
char eeprom_wifi[40];
uint8_t eeprom_wifi_len;
/***********************************************************
 * Private function prototypes
 ***********************************************************/

void cb_com_ident_changed(Proto_Num_e dummy);
void cb_com_devpar_changed(Proto_Num_e dummy);
void cb_com_kompid_changed(Proto_Num_e dummy);
void cb_com_errmsg_changed(Proto_Num_e dummy);
void cb_com_regler_changed(Proto_Num_e num);
void cb_com_diparam_changed(Proto_Num_e num);
void cb_com_measdata_changed(Proto_Num_e num);
void cb_com_status_changed(Proto_Num_e dummy);
void cb_com_msconfig_changed(Proto_Num_e dummy);
void cb_com_setz_changed(Proto_Num_e num);
void cb_com_internals_changed(Proto_Num_e num);
void cb_wlan_password_changed(Proto_Num_e dummy);
void cb_pwr_countdown_changed(Proto_Num_e dummy);
void cb_usb_stick_changed(Proto_Num_e dummy);
void cb_scriptstate_changed(Proto_Num_e dummy);
void cb_scriptcommand_changed(Proto_Num_e dummy);
void cb_local_source_changed(Proto_Num_e dummy);
void cb_com_kennlinie_changed(Proto_Num_e dummy);
void cb_com_update_changed(Proto_Num_e num);
void cb_com_reset_changed(void);
void cb_com_send_overflow(void);
void cb_com_recv_error(int8_t errnum);

void subscribe_to_com_callbacks(bool subscribe);

// V: function that will process the Kompid and get the Wifi name from it.
void process_kompid(char src[], char dest[], int);
// V: functions used to copy the processed strings
void copy_str(char *, char *, int);
void update_WLAN_password(uint32_t password);

// V: EEPROM function for master slave.
void Init_EEPROM();
void writeEEPROM(uint8_t data, uint8_t addr);
void writeEEPROM_u32(uint32_t data, uint8_t addr);
uint8_t readEEPROM(uint8_t addr);
uint32_t readEEPROM_u32(uint8_t addr);
Baudrate_e get_baudrate(uint32_t baud);


// V: EEPROM functions to read & write data related to the Wi-Fi name.
void take_care_of_wifi(void);
int find_wifi_str_len(char *);
void eeprom_write_wifi_str_char(char data, uint8_t addr);
char eeprom_read_wifi_str_char(uint8_t addr);
bool compare_wifi_strings(char *, char *, int);
void erase_eeprom_data(void);
//==============================================================
//                  SETUP
//==============================================================

// #define MEMORY_INFO
void setup(void)
{
	ESP.wdtDisable();
	ESP.wdtEnable(3000); 	// V: Enabling the watchdog with 3 sec feed.
	
	timer_init();
	lastNoClientMessage = timer_get_time();
	// Ws_last_Ping_Time = timer_get_time();
	// Ws_Ping_Time = timer_get_time();
	// msstate_timer = timer_get_time();
	// measure_timer = timer_get_time();

	Serial.begin(115200);   // V: same baud rate as the STM controller.
	Serial1.begin(115200);
	Serial.setRxBufferSize(300);	// V: increasing the size of rx buffer in UART Port.
									// V: size of UART rx buffer in STM is 300 bytes.
	Serial_Printing_Port.println("\n\n------------>   Start of the Programm. <-------------\n");
	system_update_cpu_freq(160); // V: improving clock speed to 160
	#ifdef MEMORY_INFO
	Print_system_info();
	#endif

#ifdef DEBUG_MESSAGES_ACTIVE
	Serial_Printing_Port.println("UART started");
#endif

	/* SPIFFS */
	if (!SPIFFS.begin())
	{
#ifdef DEBUG_MESSAGES_ACTIVE
		Serial_Printing_Port.println("An Error has occurred while mounting SPIFFS");
#endif
		return;
	}

	/* Basic initialization */
	COM_Driver_Init(&com_drv);		
	com_drv.fcn_ident_changed = cb_com_ident_changed;
	com_drv.fcn_kompid_changed = cb_com_kompid_changed;
	com_drv.fcn_devpar_changed = cb_com_devpar_changed;
	com_drv.fcn_measdata_changed = cb_com_measdata_changed;
	com_drv.fcn_status_changed = cb_com_status_changed;
	com_drv.fcn_setz_changed = cb_com_setz_changed;
	com_drv.fcn_internals_changed = cb_com_internals_changed;
	com_drv.fcn_local_source_changed = cb_local_source_changed;
	com_drv.fcn_update_changed = cb_com_update_changed;
	com_drv.fcn_reset_changed = cb_com_reset_changed;
	com_drv.fcn_send_overflow = cb_com_send_overflow;
	com_drv.fcn_recv_error = cb_com_recv_error;
	/* Register the driver in com module */
	COM_Driver_Register(&com_drv);

	Init_EEPROM();
	// take_care_of_wifi();
	source_local = 1; // set local source to WLAN on Dev Board
	Init_Wifi();	 
	Arduino_OTA_functionality();
	// FOTA_Through_Webpage(); 


	delay(10);
	Serial_Printing_Port.println("\nAll Initialisations completed successfully.");
	Clean_serial_buffer();	// V: clearing the Serial rx buffer for every start to avoid Tx & Rx conflicts.

	// V: requesting the required parameters for Data Synchronistaion.
	update_All_Data();

	#ifdef ELEGANT_OTA_ENABLE
		ElegantOTA.begin(&server);
		Serial1.println("Initialised with ElegantOTA.");
	#endif
	// erase_eeprom_data();
	// ESP.restart();
}

//==============================================================
//      		               LOOP
// =============================================================

void loop(void)
{
	ArduinoOTA.handle();	// V: Library function to handle FOTA updates.
	ESP.wdtFeed();			// V: Feeding the Watchdog
	COM_Handler();			// V: Data from STM32 will Be Taken Care Here (UART)
	ESP.wdtFeed();	  	
	WS_COM_Handler(); 		// V: STM data will be sent to Front-end here.

	// Counter_Handler();   // V: still Figuring out
					  		// read button function
}

//==============================================================
//                     Help Functions
//==============================================================
// total limit of decimals after decimal point in the case of a very large number in the calculation. (e.g. Umax = 10000000, TolU = 0.00000001)


#define TOL_ACCURACY_MAX 5 

uint8_t getpos_msb_digit_after_decimal_point(double val, uint32_t *multiplier)
{
	uint8_t n = 0;	 // position
	*multiplier = 1; // multiplier
	while ((uint32_t)(*multiplier * val) < 1)
	{
		*multiplier *= 10;
		if (++n > TOL_ACCURACY_MAX)
			break; // in this case the value on the display covers the labels(or it goes beyond the display border) and so the user will know that there is an error. This occurs when the passed val is 0
	}
	return n;
}

void update_Accuracy(void)
{
	double base;
	double accuracy;
	double res;
	uint32_t multipl;

	base = (double)NtParam.devparams.Umax;
	accuracy = (double)((uint32_t)(NtParam.devparams.TolU * 1000000)) / 1000000;
	res = base * accuracy;
	devParam.accuracy.digitAfterPointV = getpos_msb_digit_after_decimal_point(res, &multipl);

	base = (double)NtParam.devparams.Imax;
	accuracy = (double)((uint32_t)(NtParam.devparams.TolI * 1000000)) / 1000000;
	res = base * accuracy;
	devParam.accuracy.digitAfterPointI = getpos_msb_digit_after_decimal_point(res, &multipl);

	base = (double)NtParam.devparams.Pmax;
	accuracy = (double)((uint32_t)(NtParam.devparams.TolP * 1000000)) / 1000000;
	res = base * accuracy;
	devParam.accuracy.digitAfterPointP = getpos_msb_digit_after_decimal_point(res, &multipl);

	base = (double)NtParam.devparams.Rimax;
	accuracy = (double)((uint32_t)(NtParam.devparams.TolRi * 1000000)) / 1000000;
	res = base * accuracy;
	devParam.accuracy.digitAfterPointR = getpos_msb_digit_after_decimal_point(res, &multipl);
}

//==============================================================
//                     CALLBACKS
//==============================================================

void cb_com_ident_changed(Proto_Num_e dummy)
{
	ident.softwareId = com_drv.Ident.softwareId;
	queue_WS_MSG(SW_ID);
}

void cb_com_devpar_changed(Proto_Num_e dummy)
{
	// assign parameters to the global placeholder netzteil_parameters
	NtParam.devparams.Umax = com_drv.DevParam.umax;
	NtParam.devparams.Imax = com_drv.DevParam.imax;
	NtParam.kennlinie.MaxU = NtParam.devparams.Umax;
	NtParam.kennlinie.MaxI = NtParam.devparams.Imax;
	NtParam.devparams.Pmax = com_drv.DevParam.pmax;
	NtParam.devparams.Rimax = com_drv.DevParam.rimax;
	NtParam.devparams.Rimin = com_drv.DevParam.rimin;
	NtParam.devparams.TolU = com_drv.DevParam.tol_u;
	NtParam.devparams.TolI = com_drv.DevParam.tol_i;
	NtParam.devparams.TolP = com_drv.DevParam.tol_p;
	NtParam.devparams.TolRi = com_drv.DevParam.tol_ri;
	NtParam.devparams.Uneg = com_drv.DevParam.Uneg;
	NtParam.devparams.Ineg = com_drv.DevParam.Ineg;

	NtParam.devparams.RS232_available = (com_drv.DevParam.RS232_available >> 0) & 1; // CHECK_BIT(com_drv.DevParam.ausstattung, 0);
	NtParam.devparams.RS485_available = (com_drv.DevParam.RS485_available >> 1) & 1; // CHECK_BIT(com_drv.DevParam.ausstattung, 1);
	NtParam.devparams.LAN_available = (com_drv.DevParam.LAN_available >> 2) & 1;
	NtParam.devparams.USB_available = (com_drv.DevParam.USB_available >> 3) & 1;
	NtParam.devparams.CAN_available = (com_drv.DevParam.CAN_available >> 4) & 1;
	NtParam.devparams.GPIB_available = (com_drv.DevParam.GPIB_available >> 5) & 1;
	NtParam.devparams.MS_available = (com_drv.DevParam.MS_available >> 6) & 1;
	NtParam.devparams.AI_available = (com_drv.DevParam.AI_available >> 7) & 1;

	devParam.language = (Language_e)com_drv.DevParam.Language; // uint8_t
	// devParam.ausstattung = (uint8_t)com_drv.DevParam.ausstattung;
	devParam.Umax = com_drv.DevParam.umax;
	devParam.Imax = com_drv.DevParam.imax;
	devParam.Pmax = com_drv.DevParam.pmax;
	devParam.Rmax = com_drv.DevParam.rimax;
	devParam.Rmin = com_drv.DevParam.rimin;

	// update_Accuracy();
	queue_WS_MSG(DEV_PARAM);

	// TRIM VALUES HERE:

	/* SETVALUES */
	// set the actuall values of U/I within the limits
	if (preset.v > devParam.Umax)
	{
		preset.v = devParam.Umax;
		queue_WS_MSG(SET_V);
	}

	if (preset.i > devParam.Imax)
	{
		preset.i = devParam.Imax;
		queue_WS_MSG(SET_I);
	}

	// set the actuall values of U/I mppp within the limits
	if (preset.vmpp > devParam.Umax)
	{
		preset.vmpp = devParam.Umax;
		queue_WS_MSG(SET_VMPP);
	}

	if (preset.impp > devParam.Imax)
	{
		preset.vmpp = devParam.Imax;
		queue_WS_MSG(SET_IMPP);
	}

	// set the setvalue of Rimax within the limits
	if (preset.r > devParam.Rmax)
	{
		preset.r = devParam.Rmax;
		queue_WS_MSG(SET_R);
	}

	// set the setvalue of Rimin within the limits
	if (preset.r < devParam.Rmin)
	{
		preset.r = devParam.Rmin;
		queue_WS_MSG(SET_R);
	}

	// set the setvalue of P within the limits
	if (preset.p > devParam.Pmax)
	{
		preset.p = devParam.Pmax;
		queue_WS_MSG(SET_P);
	}

	/* PROTECTION */
	// set the setvalue of OVP within the limits
	if (preset.ovp > OVP_TOT_MAX(devParam.Umax))
	{
		preset.ovp = OVP_TOT_MAX(devParam.Umax);
		queue_WS_MSG(SET_OVP);
	}

	// set the setvalue of UVP within the limits
	if ((preset.uvp != Off) && (preset.uvp > devParam.Umax))
	{
		preset.uvp = devParam.Umax;
		queue_WS_MSG(SET_UVP);
	}

	// set the setvalue of OCP within the limits
	if ((preset.ocp != Off) && (preset.ocp > devParam.Imax))
	{
		preset.ocp = devParam.Imax;
		queue_WS_MSG(SET_OCP);
	}

	/* CONFIG */
	// set the LIM-values of  within the limits -> old
	// reset the LIM-values always when dev max parameter are changed. the Netzteil then takes care of the limits himself.
	preset.vLim = devParam.Umax;
	queue_WS_MSG(SET_VLIM);

	// if(ntparam->config.Ilim > ntparam->devparams.Imax)
	preset.iLim = devParam.Imax;
	queue_WS_MSG(SET_ILIM);

	// update_xy(GRAPH_SEND_ALL);
}

void cb_com_kompid_changed(Proto_Num_e dummy)
{
	Serial_Printing_Port.println("\nReturning from Kompid to Discard ambiguties.\n");
	return; // V: kept for unhandled cases change afterwards
	bool to_store = false;
	for (uint16_t i = 0; i < KOMP_ID_LENGTH; ++i)
	{
		compid.id[i] = com_drv.Comp_id.id[i];
	}
	process_kompid(compid.id, &komp_wifi[4], KOMP_ID_LENGTH);
	Serial_Printing_Port.print("Komp Wi-Fi string is : ");
	Serial_Printing_Port.println(komp_wifi);

	komp_wifi_len = find_wifi_str_len(komp_wifi);
	
	if(Wifi_is_stored)
	{
		// Wi-Fi string is retrived from the EEPROM. so compare and take actions accordingly.
		if( eeprom_wifi_len == komp_wifi_len)
		{
			if(compare_wifi_strings(eeprom_wifi, komp_wifi, komp_wifi_len))
			{
				to_store = false;
				Serial_Printing_Port.println("Both strings are equal.");
			}
			else
			{
				to_store = true;
			}
		}
		else
		{
			// if length is not same then no need to compare both strings
			// store the komp_wifi into EEPROM & restart the device
			to_store = true;
		}
	}
	else
	{
		to_store = true;
	}

	if(to_store)
	{
		Serial_Printing_Port.println("Kompid is changed, So we should store.");
		EEPROM.begin(EEPROM_SIZE);
		EEPROM.write(EEPROM_WIFI_LEN_ADDRESS, komp_wifi_len);
		for (uint8_t i = 0; i < komp_wifi_len; i++)
		{
			eeprom_write_wifi_str_char(komp_wifi[i], i + EEPROM_WIFI_STR_ADDRESS);
		}
		Serial_Printing_Port.println("Restarting the Wi-Fi module.");
		EEPROM.end(); 
		ESP.restart();
	}

	Wifi_ready = true;
	EEPROM.end(); // V: calling this to successfully release the EEPROM emulation from RAM
	// queue_WS_MSG(CompID);
}

// V: function to process the kompid and get the perfect device name from it.
// LAb-Hp \n11.12.13\nvLp:␎tZ:␜{':␛/�;
void process_kompid(char src[], char dest[], int len)
{
	// V: local variables for storing processed strings before storing into destination
	int i=0, j=0, c=0; // V: i & j acts as pointers that will travel accross the string,, c will be holding the length of processed strings.

	if(src[0] == ' ')
	{
		i=1;
		j=1;
	}

	for(; j<len; j++)
	{
		if((src[j] == ' ') || ((src[j] >= '0') && (src[j] <= '9')))
		{
			copy_str(&src[i], &dest[c], j-i); // LAB-HP string
			c = j-i;
			while((j < len) && ((src[j] != '\\') && (src[j+1] != 'n')))
			{
				j++;
			}
			j = j+2;
			i = j;
			break;
		}
		else if(((src[j] == '\\') && (src[j+1] == 'n')))
		{
			copy_str(&src[i], &dest[c], j-i); // LAB-HP string
			c = j-i;
			j = j+2;
			i = j;
			break;
		}
	}

	dest[c] = '-';
	c++;

	while((j < len) && ((src[j] != '\\') && (src[j+1] != 'n')))
	{
		j++;
	}
	copy_str(&src[i], &dest[c], j-i);
	c = c + (j-i); // calculating whole length of the string.
	dest[c] = '\0';
}

void copy_str(char src[], char dest[], int len)
{
	for (int i=0; i<len; i++)
	{
		dest[i] = src[i];
	}
}


void cb_com_errmsg_changed(Proto_Num_e dummy)
{
	NtParam.errmsg.errortype = (ErrMsgType_e)com_drv.ErrMessage.Errortype;
	for (uint8_t i = 0; i < 78; ++i)
	{
		NtParam.errmsg.msg[i] = com_drv.ErrMessage.Message[i];
	}
	if (NtParam.errmsg.msg[0] == 0xE6)
		queue_WS_MSG(USB_ERR);
	else
		queue_WS_MSG(ERR_MSG);
}

void cb_com_regler_changed(Proto_Num_e num)
{
	if (num == REGLER_PM)
	{
		control.p.p = com_drv.Regler_pm.P;
		control.p.i = com_drv.Regler_pm.I;
		control.p.d = com_drv.Regler_pm.D;
		queue_WS_MSG(CONTROL_P);
	}
	else if (num == REGLER_RIM)
	{
		control.ri.p = com_drv.Regler_rim.P;
		control.ri.i = com_drv.Regler_rim.I;
		control.ri.d = com_drv.Regler_rim.D;
		queue_WS_MSG(CONTROL_RI);
	}
	else if (num == REGLER_PVM)
	{
		control.pv.p = com_drv.Regler_pvm.P;
		control.pv.i = com_drv.Regler_pvm.I;
		control.pv.d = com_drv.Regler_pvm.D;
		queue_WS_MSG(CONTROL_PV);
	}
}

void cb_com_diparam_changed(Proto_Num_e num)
{

	switch (num)
	{

	case PARAM_DI1:
	{
		NtParam.interface[0].type = (IfType_e)com_drv.Param_di1.Typ;
		NtParam.interface[0].Addr = com_drv.Param_di1.Adresse;
		NtParam.interface[0].TO = com_drv.Param_di1.Timeout;
		NtParam.interface[0].databits = (Databits_e)com_drv.Param_di1.Datenbits;
		NtParam.interface[0].stopbits = (Stopbits_e)com_drv.Param_di1.Stopbits;
		NtParam.interface[0].parity = (Parity_e)com_drv.Param_di1.Parity;
		NtParam.interface[0].echo = (Echo_e)com_drv.Param_di1.Echo;
		NtParam.interface[0].handshake = (Handshake_e)com_drv.Param_di1.Handshake;
		NtParam.interface[0].baudrate = get_baudrate(com_drv.Param_di1.baud);

		queue_WS_MSG(DI_PARAM1);
		break;
	}

	case PARAM_DI2:
	{
		NtParam.interface[1].type = (IfType_e)com_drv.Param_di2.Typ;
		NtParam.interface[1].Addr = com_drv.Param_di2.Adresse;
		NtParam.interface[1].TO = com_drv.Param_di2.Timeout;
		NtParam.interface[1].databits = (Databits_e)com_drv.Param_di2.Datenbits;
		NtParam.interface[1].stopbits = (Stopbits_e)com_drv.Param_di2.Stopbits;
		NtParam.interface[1].parity = (Parity_e)com_drv.Param_di2.Parity;
		NtParam.interface[1].echo = (Echo_e)com_drv.Param_di2.Echo;
		NtParam.interface[1].handshake = (Handshake_e)com_drv.Param_di2.Handshake;
		NtParam.interface[1].baudrate = get_baudrate(com_drv.Param_di2.baud);

		queue_WS_MSG(DI_PARAM2);
		break;
	}

	case PARAM_DI3:
	{
		NtParam.interface[2].type = (IfType_e)com_drv.Param_di3.Typ;
		NtParam.interface[2].Addr = com_drv.Param_di3.Adresse;
		NtParam.interface[2].TO = com_drv.Param_di3.Timeout;
		NtParam.interface[2].databits = (Databits_e)com_drv.Param_di3.Datenbits;
		NtParam.interface[2].stopbits = (Stopbits_e)com_drv.Param_di3.Stopbits;
		NtParam.interface[2].parity = (Parity_e)com_drv.Param_di3.Parity;
		NtParam.interface[2].echo = (Echo_e)com_drv.Param_di3.Echo;
		NtParam.interface[2].handshake = (Handshake_e)com_drv.Param_di3.Handshake;
		NtParam.interface[2].baudrate = get_baudrate(com_drv.Param_di3.baud);

		queue_WS_MSG(DI_PARAM3);
		break;
	}

	case PARAM_DI4:
	{
		NtParam.interface[3].type = (IfType_e)com_drv.Param_di4.Typ;
		NtParam.interface[3].Addr = com_drv.Param_di4.Adresse;
		NtParam.interface[3].TO = com_drv.Param_di4.Timeout;
		NtParam.interface[3].databits = (Databits_e)com_drv.Param_di4.Datenbits;
		NtParam.interface[3].stopbits = (Stopbits_e)com_drv.Param_di4.Stopbits;
		NtParam.interface[3].parity = (Parity_e)com_drv.Param_di4.Parity;
		NtParam.interface[3].echo = (Echo_e)com_drv.Param_di4.Echo;
		NtParam.interface[3].handshake = (Handshake_e)com_drv.Param_di4.Handshake;
		NtParam.interface[3].baudrate = get_baudrate(com_drv.Param_di4.baud);

		queue_WS_MSG(DI_PARAM4);
		break;
	}

	case PARAM_DI5:
	{
		NtParam.interface[4].type = (IfType_e)com_drv.Param_di5.Typ;
		NtParam.interface[4].Addr = com_drv.Param_di5.Adresse;
		NtParam.interface[4].TO = com_drv.Param_di5.Timeout;
		NtParam.interface[4].databits = (Databits_e)com_drv.Param_di5.Datenbits;
		NtParam.interface[4].stopbits = (Stopbits_e)com_drv.Param_di5.Stopbits;
		NtParam.interface[4].parity = (Parity_e)com_drv.Param_di5.Parity;
		NtParam.interface[4].echo = (Echo_e)com_drv.Param_di5.Echo;
		NtParam.interface[4].handshake = (Handshake_e)com_drv.Param_di5.Handshake;
		NtParam.interface[4].baudrate = get_baudrate(com_drv.Param_di5.baud);

		queue_WS_MSG(DI_PARAM5);
		break;
	}

	case PARAM_DI6:
	{
		NtParam.interface[5].type = (IfType_e)com_drv.Param_di6.Typ;
		NtParam.interface[5].Addr = com_drv.Param_di6.Adresse;
		NtParam.interface[5].TO = com_drv.Param_di6.Timeout;
		NtParam.interface[5].databits = (Databits_e)com_drv.Param_di6.Datenbits;
		NtParam.interface[5].stopbits = (Stopbits_e)com_drv.Param_di6.Stopbits;
		NtParam.interface[5].parity = (Parity_e)com_drv.Param_di6.Parity;
		NtParam.interface[5].echo = (Echo_e)com_drv.Param_di6.Echo;
		NtParam.interface[5].handshake = (Handshake_e)com_drv.Param_di6.Handshake;
		NtParam.interface[5].baudrate = get_baudrate(com_drv.Param_di6.baud);

		queue_WS_MSG(DI_PARAM6);
		break;
	}

	case PARAM_DI7:
	{
		NtParam.interface[6].type = (IfType_e)com_drv.Param_di7.Typ;
		NtParam.interface[6].Addr = com_drv.Param_di7.Adresse;
		NtParam.interface[6].TO = com_drv.Param_di7.Timeout;
		NtParam.interface[6].databits = (Databits_e)com_drv.Param_di7.Datenbits;
		NtParam.interface[6].stopbits = (Stopbits_e)com_drv.Param_di7.Stopbits;
		NtParam.interface[6].parity = (Parity_e)com_drv.Param_di7.Parity;
		NtParam.interface[6].echo = (Echo_e)com_drv.Param_di7.Echo;
		NtParam.interface[6].handshake = (Handshake_e)com_drv.Param_di7.Handshake;
		NtParam.interface[6].baudrate = get_baudrate(com_drv.Param_di7.baud);

		queue_WS_MSG(DI_PARAM7);
		break;
	}

	case PARAM_DI8:
	{
		NtParam.interface[7].type = (IfType_e)com_drv.Param_di8.Typ;
		NtParam.interface[7].Addr = com_drv.Param_di8.Adresse;
		NtParam.interface[7].TO = com_drv.Param_di8.Timeout;
		NtParam.interface[7].databits = (Databits_e)com_drv.Param_di8.Datenbits;
		NtParam.interface[7].stopbits = (Stopbits_e)com_drv.Param_di8.Stopbits;
		NtParam.interface[7].parity = (Parity_e)com_drv.Param_di8.Parity;
		NtParam.interface[7].echo = (Echo_e)com_drv.Param_di8.Echo;
		NtParam.interface[7].handshake = (Handshake_e)com_drv.Param_di8.Handshake;
		NtParam.interface[7].baudrate = get_baudrate(com_drv.Param_di8.baud);

		queue_WS_MSG(DI_PARAM8);
		break;
	}

	case PARAM_DI9:
	{
		NtParam.interface[8].type = (IfType_e)com_drv.Param_di9.Typ;
		NtParam.interface[8].Addr = com_drv.Param_di9.Adresse;
		NtParam.interface[8].TO = com_drv.Param_di9.Timeout;
		NtParam.interface[8].databits = (Databits_e)com_drv.Param_di9.Datenbits;
		NtParam.interface[8].stopbits = (Stopbits_e)com_drv.Param_di9.Stopbits;
		NtParam.interface[8].parity = (Parity_e)com_drv.Param_di9.Parity;
		NtParam.interface[8].echo = (Echo_e)com_drv.Param_di9.Echo;
		NtParam.interface[8].handshake = (Handshake_e)com_drv.Param_di9.Handshake;
		NtParam.interface[8].baudrate = get_baudrate(com_drv.Param_di9.baud);

		queue_WS_MSG(DI_PARAM9);
		break;
	}

	case PARAM_DI10:
	{
		NtParam.interface[9].type = (IfType_e)com_drv.Param_di10.Typ;
		NtParam.interface[9].Addr = com_drv.Param_di10.Adresse;
		NtParam.interface[9].TO = com_drv.Param_di10.Timeout;
		NtParam.interface[9].databits = (Databits_e)com_drv.Param_di10.Datenbits;
		NtParam.interface[9].stopbits = (Stopbits_e)com_drv.Param_di10.Stopbits;
		NtParam.interface[9].parity = (Parity_e)com_drv.Param_di10.Parity;
		NtParam.interface[9].echo = (Echo_e)com_drv.Param_di10.Echo;
		NtParam.interface[9].handshake = (Handshake_e)com_drv.Param_di10.Handshake;
		NtParam.interface[9].baudrate = get_baudrate(com_drv.Param_di10.baud);

		queue_WS_MSG(DI_PARAM10);
		break;
	}

	case PARAM_DI11:
	{
		NtParam.interface[10].type = (IfType_e)com_drv.Param_di11.Typ;
		NtParam.interface[10].Addr = com_drv.Param_di11.Adresse;
		NtParam.interface[10].TO = com_drv.Param_di11.Timeout;
		NtParam.interface[10].databits = (Databits_e)com_drv.Param_di11.Datenbits;
		NtParam.interface[10].stopbits = (Stopbits_e)com_drv.Param_di11.Stopbits;
		NtParam.interface[10].parity = (Parity_e)com_drv.Param_di11.Parity;
		NtParam.interface[10].echo = (Echo_e)com_drv.Param_di11.Echo;
		NtParam.interface[10].handshake = (Handshake_e)com_drv.Param_di11.Handshake;
		NtParam.interface[10].baudrate = get_baudrate(com_drv.Param_di11.baud);

		queue_WS_MSG(DI_PARAM11);
		break;
	}

	case PARAM_DI12:
	{
		NtParam.interface[11].type = (IfType_e)com_drv.Param_di12.Typ;
		NtParam.interface[11].Addr = com_drv.Param_di12.Adresse;
		NtParam.interface[11].TO = com_drv.Param_di12.Timeout;
		NtParam.interface[11].databits = (Databits_e)com_drv.Param_di12.Datenbits;
		NtParam.interface[11].stopbits = (Stopbits_e)com_drv.Param_di12.Stopbits;
		NtParam.interface[11].parity = (Parity_e)com_drv.Param_di12.Parity;
		NtParam.interface[11].echo = (Echo_e)com_drv.Param_di12.Echo;
		NtParam.interface[11].handshake = (Handshake_e)com_drv.Param_di12.Handshake;
		NtParam.interface[11].baudrate = get_baudrate(com_drv.Param_di12.baud);

		queue_WS_MSG(DI_PARAM12);
		break;
	}

	case PARAM_DI13:
	{
		NtParam.interface[12].type = (IfType_e)com_drv.Param_di13.Typ;
		NtParam.interface[12].Addr = com_drv.Param_di13.Adresse;
		NtParam.interface[12].TO = com_drv.Param_di13.Timeout;
		NtParam.interface[12].databits = (Databits_e)com_drv.Param_di13.Datenbits;
		NtParam.interface[12].stopbits = (Stopbits_e)com_drv.Param_di13.Stopbits;
		NtParam.interface[12].parity = (Parity_e)com_drv.Param_di13.Parity;
		NtParam.interface[12].echo = (Echo_e)com_drv.Param_di13.Echo;
		NtParam.interface[12].handshake = (Handshake_e)com_drv.Param_di13.Handshake;
		NtParam.interface[12].baudrate = get_baudrate(com_drv.Param_di13.baud);

		queue_WS_MSG(DI_PARAM13);
		break;
	}

	case PARAM_DI14:
	{
		NtParam.interface[13].type = (IfType_e)com_drv.Param_di14.Typ;
		NtParam.interface[13].Addr = com_drv.Param_di14.Adresse;
		NtParam.interface[13].TO = com_drv.Param_di14.Timeout;
		NtParam.interface[13].databits = (Databits_e)com_drv.Param_di14.Datenbits;
		NtParam.interface[13].stopbits = (Stopbits_e)com_drv.Param_di14.Stopbits;
		NtParam.interface[13].parity = (Parity_e)com_drv.Param_di14.Parity;
		NtParam.interface[13].echo = (Echo_e)com_drv.Param_di14.Echo;
		NtParam.interface[13].handshake = (Handshake_e)com_drv.Param_di14.Handshake;
		NtParam.interface[13].baudrate = get_baudrate(com_drv.Param_di14.baud);

		queue_WS_MSG(DI_PARAM14);
		break;
	}

	case PARAM_DI15:
	{
		NtParam.interface[14].type = (IfType_e)com_drv.Param_di15.Typ;
		NtParam.interface[14].Addr = com_drv.Param_di15.Adresse;
		NtParam.interface[14].TO = com_drv.Param_di15.Timeout;
		NtParam.interface[14].databits = (Databits_e)com_drv.Param_di15.Datenbits;
		NtParam.interface[14].stopbits = (Stopbits_e)com_drv.Param_di15.Stopbits;
		NtParam.interface[14].parity = (Parity_e)com_drv.Param_di15.Parity;
		NtParam.interface[14].echo = (Echo_e)com_drv.Param_di15.Echo;
		NtParam.interface[14].handshake = (Handshake_e)com_drv.Param_di15.Handshake;
		NtParam.interface[14].baudrate = get_baudrate(com_drv.Param_di15.baud);

		queue_WS_MSG(DI_PARAM15);
		break;
	}

	case PARAM_DI16:
	{
		NtParam.interface[15].type = (IfType_e)com_drv.Param_di16.Typ;
		NtParam.interface[15].Addr = com_drv.Param_di16.Adresse;
		NtParam.interface[15].TO = com_drv.Param_di16.Timeout;
		NtParam.interface[15].databits = (Databits_e)com_drv.Param_di16.Datenbits;
		NtParam.interface[15].stopbits = (Stopbits_e)com_drv.Param_di16.Stopbits;
		NtParam.interface[15].parity = (Parity_e)com_drv.Param_di16.Parity;
		NtParam.interface[15].echo = (Echo_e)com_drv.Param_di16.Echo;
		NtParam.interface[15].handshake = (Handshake_e)com_drv.Param_di16.Handshake;
		NtParam.interface[15].baudrate = get_baudrate(com_drv.Param_di16.baud);
		queue_WS_MSG(DI_PARAM16);
		break;
	}
	default:
		break;
	}
}

void cb_com_measdata_changed(Proto_Num_e num)
{
	uint8_t id = 0xFF;

	if (num == MESSDATEN)
	{
		NtParam.monitor.U = com_drv.Messdaten.Ua;
		NtParam.monitor.I = com_drv.Messdaten.Ia;
		NtParam.monitor.P = com_drv.Messdaten.Pa;

		if (NtParam.monitor.I == 0) // zero division check
			NtParam.monitor.R = 0;
		else
			NtParam.monitor.R = NtParam.monitor.U / NtParam.monitor.I;

		// Copying to monitor struct, sending to WS & Returning from here.
		monitor.v = NtParam.monitor.U;
		monitor.i = NtParam.monitor.I;
		monitor.p = NtParam.monitor.P;
		monitor.r = NtParam.monitor.R;
		queue_WS_MSG(MONVAL);
		return;
	}
	else if (num == MESS_MS1)
	{
		id = 0;
		NtParam.msmonitor[0].U = com_drv.Mess_ms1.Ua;
		NtParam.msmonitor[0].I = com_drv.Mess_ms1.Ia;
		NtParam.msmonitor[0].P = com_drv.Mess_ms1.Pa;
		NtParam.msmonitor[0].R =
			(NtParam.msmonitor[0].I != 0) ? NtParam.msmonitor[0].U / NtParam.msmonitor[0].I : 0;
	}
	else if (num == MESS_MS2)
	{
		id = 1;
		NtParam.msmonitor[1].U = com_drv.Mess_ms2.Ua;
		NtParam.msmonitor[1].I = com_drv.Mess_ms2.Ia;
		NtParam.msmonitor[1].P = com_drv.Mess_ms2.Pa;
		NtParam.msmonitor[1].R =
			(NtParam.msmonitor[1].I != 0) ? NtParam.msmonitor[1].U / NtParam.msmonitor[1].I : 0;
	}
	else if (num == MESS_MS3)
	{
		id = 2;
		NtParam.msmonitor[2].U = com_drv.Mess_ms3.Ua;
		NtParam.msmonitor[2].I = com_drv.Mess_ms3.Ia;
		NtParam.msmonitor[2].P = com_drv.Mess_ms3.Pa;
		NtParam.msmonitor[2].R =
			(NtParam.msmonitor[2].I != 0) ? NtParam.msmonitor[2].U / NtParam.msmonitor[2].I : 0;
	}
	else if (num == MESS_MS4)
	{
		id = 3;
		NtParam.msmonitor[3].U = com_drv.Mess_ms4.Ua;
		NtParam.msmonitor[3].I = com_drv.Mess_ms4.Ia;
		NtParam.msmonitor[3].P = com_drv.Mess_ms4.Pa;
		NtParam.msmonitor[3].R =
			(NtParam.msmonitor[3].I != 0) ? NtParam.msmonitor[3].U / NtParam.msmonitor[3].I : 0;
	}
	else if (num == MESS_MS5)
	{
		id = 4;
		NtParam.msmonitor[4].U = com_drv.Mess_ms5.Ua;
		NtParam.msmonitor[4].I = com_drv.Mess_ms5.Ia;
		NtParam.msmonitor[4].P = com_drv.Mess_ms5.Pa;
		NtParam.msmonitor[4].R =
			(NtParam.msmonitor[4].I != 0) ? NtParam.msmonitor[4].U / NtParam.msmonitor[4].I : 0;
	}
	else if (num == MESS_MS6)
	{
		id = 5;
		NtParam.msmonitor[5].U = com_drv.Mess_ms6.Ua;
		NtParam.msmonitor[5].I = com_drv.Mess_ms6.Ia;
		NtParam.msmonitor[5].P = com_drv.Mess_ms6.Pa;
		NtParam.msmonitor[5].R =
			(NtParam.msmonitor[5].I != 0) ? NtParam.msmonitor[5].U / NtParam.msmonitor[5].I : 0;
	}
	else if (num == MESS_MS7)
	{
		id = 6;
		NtParam.msmonitor[6].U = com_drv.Mess_ms7.Ua;
		NtParam.msmonitor[6].I = com_drv.Mess_ms7.Ia;
		NtParam.msmonitor[6].P = com_drv.Mess_ms7.Pa;
		NtParam.msmonitor[6].R =
			(NtParam.msmonitor[6].I != 0) ? NtParam.msmonitor[6].U / NtParam.msmonitor[6].I : 0;
	}
	else if (num == MESS_MS8)
	{
		id = 7;
		NtParam.msmonitor[7].U = com_drv.Mess_ms8.Ua;
		NtParam.msmonitor[7].I = com_drv.Mess_ms8.Ia;
		NtParam.msmonitor[7].P = com_drv.Mess_ms8.Pa;
		NtParam.msmonitor[7].R =
			(NtParam.msmonitor[7].I != 0) ? NtParam.msmonitor[7].U / NtParam.msmonitor[7].I : 0;
	}
	else if (num == MESS_MS9)
	{
		id = 8;
		NtParam.msmonitor[8].U = com_drv.Mess_ms9.Ua;
		NtParam.msmonitor[8].I = com_drv.Mess_ms9.Ia;
		NtParam.msmonitor[8].P = com_drv.Mess_ms9.Pa;
		NtParam.msmonitor[8].R =
			(NtParam.msmonitor[8].I != 0) ? NtParam.msmonitor[8].U / NtParam.msmonitor[8].I : 0;
	}
	else if (num == MESS_MS10)
	{
		id = 9;
		NtParam.msmonitor[9].U = com_drv.Mess_ms10.Ua;
		NtParam.msmonitor[9].I = com_drv.Mess_ms10.Ia;
		NtParam.msmonitor[9].P = com_drv.Mess_ms10.Pa;
		NtParam.msmonitor[9].R =
			(NtParam.msmonitor[9].I != 0) ? NtParam.msmonitor[9].U / NtParam.msmonitor[9].I : 0;
	}
	else if (num == MESS_MS11)
	{
		id = 10;
		NtParam.msmonitor[10].U = com_drv.Mess_ms11.Ua;
		NtParam.msmonitor[10].I = com_drv.Mess_ms11.Ia;
		NtParam.msmonitor[10].P = com_drv.Mess_ms11.Pa;
		NtParam.msmonitor[10].R =
			(NtParam.msmonitor[10].I != 0) ? NtParam.msmonitor[10].U / NtParam.msmonitor[10].I : 0;
	}
	else if (num == MESS_MS12)
	{
		id = 11;
		NtParam.msmonitor[11].U = com_drv.Mess_ms12.Ua;
		NtParam.msmonitor[11].I = com_drv.Mess_ms12.Ia;
		NtParam.msmonitor[11].P = com_drv.Mess_ms12.Pa;
		NtParam.msmonitor[11].R =
			(NtParam.msmonitor[11].I != 0) ? NtParam.msmonitor[11].U / NtParam.msmonitor[11].I : 0;
	}
	else if (num == MESS_MS13)
	{
		id = 12;
		NtParam.msmonitor[12].U = com_drv.Mess_ms13.Ua;
		NtParam.msmonitor[12].I = com_drv.Mess_ms13.Ia;
		NtParam.msmonitor[12].P = com_drv.Mess_ms13.Pa;
		NtParam.msmonitor[12].R =
			(NtParam.msmonitor[12].I != 0) ? NtParam.msmonitor[12].U / NtParam.msmonitor[12].I : 0;
	}
	else if (num == MESS_MS14)
	{
		id = 13;
		NtParam.msmonitor[13].U = com_drv.Mess_ms14.Ua;
		NtParam.msmonitor[13].I = com_drv.Mess_ms14.Ia;
		NtParam.msmonitor[13].P = com_drv.Mess_ms14.Pa;
		NtParam.msmonitor[13].R =
			(NtParam.msmonitor[13].I != 0) ? NtParam.msmonitor[13].U / NtParam.msmonitor[13].I : 0;
	}
	else if (num == MESS_MS15)
	{
		id = 14;
		NtParam.msmonitor[14].U = com_drv.Mess_ms15.Ua;
		NtParam.msmonitor[14].I = com_drv.Mess_ms15.Ia;
		NtParam.msmonitor[14].P = com_drv.Mess_ms15.Pa;
		NtParam.msmonitor[14].R =
			(NtParam.msmonitor[14].I != 0) ? NtParam.msmonitor[14].U / NtParam.msmonitor[14].I : 0;
	}
	else if (num == MESS_MS16)
	{
		id = 15;
		NtParam.msmonitor[15].U = com_drv.Mess_ms16.Ua;
		NtParam.msmonitor[15].I = com_drv.Mess_ms16.Ia;
		NtParam.msmonitor[15].P = com_drv.Mess_ms16.Pa;
		NtParam.msmonitor[15].R =
			(NtParam.msmonitor[15].I != 0) ? NtParam.msmonitor[15].U / NtParam.msmonitor[15].I : 0;
	}

	GetMonitorTotalValues(&monitor.v, &monitor.i, &monitor.p, &monitor.r);
	if (timer_get_time() - measure_timer > 100)
	{
		queue_WS_MSG(MONVAL);
		measure_timer = timer_get_time();

		if (id < 16)
		{
			queue_WS_MSG((Messages_e)(MS_TBL_0 + id));
			// ws_Send_Data((Messages_e)(MS_TBL_0 + id));
		}
	}
}

void cb_com_status_changed(Proto_Num_e dummy)
{
	status.source = (Source_e)com_drv.Status.Source;
	status.power = com_drv.Status.Standby;
	status.mode = com_drv.Status.Mode;
	status.limMode = com_drv.Status.LimMode;
	status.interlock = com_drv.Status.Interlock;
	status.interlockMode = com_drv.Status.Interlock_Mode;
	status.ReqToLocal = com_drv.Status.GoToLocal;
	status.LocalLockout = com_drv.Status.LocalLockout;

	source_local = com_drv.local_source;
	// V:checking the Fault detection bit and updating according in the Status Struct.
	if (com_drv.Status.Ovp)
	{
		status.fault = Ovp;
	}
	else if (com_drv.Status.Ocp)
	{
		status.fault = Ocp;
	}
	else if (com_drv.Status.Uvp)
	{
		status.fault = Uvp;
	}
	else if (com_drv.Status.statusOverTemp)
	{
		status.fault = Otp;
	}
	else
	{
		status.fault = NA;
	}

	if (source_local == WLAN_Source)
	{
		if (status.power == ON || status.power == OFF)
		{ // should always be true
			counter_active = false;
		}
	}
	// //get UI Curve if mode is PVSIM or USER on Restart
	if (first_status)
	{
		first_status = false;
		if (status.mode == PVSIM || status.mode == USER)
		{
			COM_GetParameter(UI_CURVE);
		}
	}
	queue_WS_MSG(NT_STATE);
	// update_xy(GRAPH_SEND_ALL);
}

void cb_com_msconfig_changed(Proto_Num_e dummy)
{
	bool msconfig_send = false;
	// ws_Send_Text("ESP (cb_msconfig)");
	NtParam.msconfig.ms_mode = (MasterSlaveMode_e)com_drv.MsConfig.Mode;
	NtParam.msconfig.Nr = com_drv.MsConfig.Nr;
	memcpy(&NtParam.msconfig.ms_net_info, (uint16_t *)&com_drv.MsConfig + 1, sizeof(uint16_t));

	if (!ms_available)
	{
		if (NtParam.msconfig.ms_mode)
		{
			ms_available = true;
			// writeEEPROM(true, 0);
		}
	}

	if (msconfig_cpy.ms_mode != NtParam.msconfig.ms_mode)
	{
		msconfig_cpy.ms_mode = NtParam.msconfig.ms_mode;
		msconfig_send = true;
	}

	if (msconfig_cpy.Nr != NtParam.msconfig.Nr)
	{
		msconfig_cpy.Nr = NtParam.msconfig.Nr;
		msconfig_send = true;
	}

	// send msconfig if values changed
	if (msconfig_send)
	{
		queue_WS_MSG(MS_STATE);
		msstate_timer = timer_get_time();
		// Serial.print("2");
	}
	else if (timer_get_time() - msstate_timer > 100)
	{ // send msconfig every 50ms
		queue_WS_MSG(MS_STATE);
		msstate_timer = timer_get_time();
	}
}

void cb_com_setz_changed(Proto_Num_e num)
{
	// uint8_t buffer[400];

	switch (num)
	{
	case U_SETZ:
		preset.v = com_drv.U_setz;
		queue_WS_MSG(SET_V);
		break;

	case I_SETZ:
		preset.i = com_drv.I_setz;
		queue_WS_MSG(SET_I);
		break;

	case UMPP_SETZ:
		preset.vmpp = com_drv.Umpp_setz;
		queue_WS_MSG(SET_VMPP);
		break;

	case IMPP_SETZ:
		preset.impp = com_drv.Impp_setz;
		queue_WS_MSG(SET_IMPP);
		break;

	case P_SETZ:
		preset.p = com_drv.P_setz;
		queue_WS_MSG(SET_P);
		break;

	case RI_SETZ:
		preset.r = com_drv.Ri_setz;
		queue_WS_MSG(SET_R);
		break;

	case AI_FILTER:
		// ntparam->interfaceai.AI_Filter = (uint32_t)com_drv.Ai_filter;
		break;

	case OVP:
		preset.ovp = com_drv.Ovp;
		queue_WS_MSG(SET_OVP);
		break;

	case OCP:
		if (isnanf(com_drv.Ocp))
			preset.ocp = -1;
		else
			preset.ocp = com_drv.Ocp;
		queue_WS_MSG(SET_OCP);
		break;

	case T_OCP:
		preset.ocpT = com_drv.T_ocp * 1000; // convert it from [s] to [ms]
		queue_WS_MSG(SET_OCPT);
		break;

	case UVP:
		if (isnanf(com_drv.Uvp))
			preset.uvp = -1;
		else
			preset.uvp = com_drv.Uvp;
		queue_WS_MSG(SET_UVP);
		break;

	case T_UVP:
		preset.uvpT = com_drv.T_uvp * 1000; // convert it from [s] to [ms]
		queue_WS_MSG(SET_UVPT);
		break;

	case U_SLOPE:
		if (isnanf(com_drv.U_slope))
			preset.vSlope = 0;
		else
			preset.vSlope = com_drv.U_slope;
		queue_WS_MSG(SET_V_SL);
		break;

	case I_SLOPE:
		if (isnanf(com_drv.I_slope))
			preset.iSlope = 0;
		else
			preset.iSlope = com_drv.I_slope;
		queue_WS_MSG(SET_I_SL);
		break;

	case U_LIMIT:
		preset.vLim = com_drv.U_limit;
		queue_WS_MSG(SET_VLIM);
		break;

	case I_LIMIT:
		preset.iLim = com_drv.I_limit;
		queue_WS_MSG(SET_ILIM);
		break;
	
	case FOLDBACK:
		FoldBack.foldback = com_drv.foldback;
		queue_WS_MSG(FOLDBACK_WS);
		break;
	
	case FOLDBACKTM:
		// com_drv.foldbacktm = com_drv.foldbacktm * 1000;
		FoldBack.foldbackTm = com_drv.foldbacktm;
		queue_WS_MSG(FOLDBACKTM_WS);
		break;

	default:
		break;
	}
}

void cb_com_internals_changed(Proto_Num_e dummy)
{

	switch (dummy)
	{
	case RELASE:
		// Remember last Settings
		intParam.remember = (bool)com_drv.Relase;
		queue_WS_MSG(REMEMBER);
		break;

	case ODELAY:
		// Output on Delay
		if (com_drv.oDelay == 255)
			intParam.Tdelay = 0;
		else
			intParam.Tdelay = (uint8_t)com_drv.oDelay;
		queue_WS_MSG(TDELAY);
		break;

	case TEN:
		// T Enable
		if (com_drv.tEn == 255)
			intParam.Tenable = 0;
		else
			intParam.Tenable = (uint8_t)com_drv.tEn;
		queue_WS_MSG(TENABLE);
		break;

	case DATALOGGER:
		// Datalogger
		if (com_drv.datalogger == 255)
			intParam.Logger = 0;
		else
			intParam.Logger = (uint8_t)com_drv.datalogger;
		queue_WS_MSG(LOGGER);
		break;

	default:
		break;
	}
}

void cb_com_kennlinie_changed(Proto_Num_e dummy)
{
	memcpy(&NtParam.kennlinie, &com_drv.Ui_curve, sizeof(T_Kennlinie));

	// update_xy(GRAPH_SEND_ALL);
}

void cb_wlan_password_changed(Proto_Num_e dummy)
{
#ifndef IS_ESP_DEV_BOARD
	NtParam.wlanconfig.password = com_drv.wlan_pw;
	update_WLAN_password(NtParam.wlanconfig.password);
#ifdef DEBUG_MESSAGES_ACTIVE
	Serial.println("New password: " + (String)WiFi.softAPPSK());
#endif
#endif
	return;
}

void cb_pwr_countdown_changed(Proto_Num_e dummy)
{
	// prio_cnt++;
	count_pwr = com_drv.pwrCountdown;
	if (count_pwr == 0)
	{
		if (source_local == SOURCE_FRONT)
		{
			if (status.power == OFF)
			{
				counter_active = false;
			}
		}
	}
	queue_WS_MSG(COUNTDOWN_PWR);
}

void cb_usb_stick_changed(Proto_Num_e dummy)
{
	usb_active = (bool)com_drv.usb_stick;
	queue_WS_MSG(USB_ACTIVE);
}

void cb_scriptstate_changed(Proto_Num_e dummy)
{
	state_script = (scriptmode_state_e)com_drv.scriptstate;
	queue_WS_MSG(STATE_SCRIPT);
	if (state_script == SCRIPTSTATE_HALT)
		COM_GetParameter(NT_STATUS);
}

void cb_scriptcommand_changed(Proto_Num_e dummy)
{
	memcpy(&cmd_script, &com_drv.scr_cmd, sizeof(T_Script_command));
	queue_WS_MSG(COMMAND_SCRIPT);
}

void cb_local_source_changed(Proto_Num_e dummy)
{
	source_local = (uint8_t)com_drv.local_source;

	// if (source_local == SOURCE_FRONT)
	// {
	// 	//
	// }
	// else if (source_local == SOURCE_WLAN)
	// {
	// 	//
	// }
	queue_WS_MSG(SOURCE_LOCAL);
}

void cb_com_send_overflow(void) {}
void cb_com_recv_error(int8_t errnum) {}
void cb_com_update_changed(Proto_Num_e num) {}
void cb_com_reset_changed(void) {}

// subscribe to callbacks of Touch front
void subscribe_to_com_callbacks(bool subscribe)
{
	if (subscribe)
	{
		com_drv.fcn_status_changed = cb_com_status_changed;
		com_drv.fcn_devpar_changed = cb_com_devpar_changed;
		com_drv.fcn_regler_changed = cb_com_regler_changed;
		com_drv.fcn_msconfig_changed = cb_com_msconfig_changed;
		com_drv.fcn_setz_changed = cb_com_setz_changed;
		com_drv.fcn_internals_changed = cb_com_internals_changed;
		com_drv.fcn_diparam_changed = cb_com_diparam_changed;
		com_drv.fcn_pwr_countdown_changed = cb_pwr_countdown_changed;
		com_drv.fcn_scriptstate_changed = cb_scriptstate_changed;
		com_drv.fcn_scriptcommand_changed = cb_scriptcommand_changed;
		com_drv.fcn_wlan_password_changed = cb_wlan_password_changed;
		// com_drv.fcn_wlan_client_connected = cb_com_client_connected;
	}
	else
	{
		com_drv.fcn_status_changed = NULL;
		com_drv.fcn_devpar_changed = NULL;
		com_drv.fcn_regler_changed = NULL;
		com_drv.fcn_msconfig_changed = NULL;
		com_drv.fcn_setz_changed = NULL;
		com_drv.fcn_internals_changed = NULL;
		com_drv.fcn_diparam_changed = NULL;
		com_drv.fcn_pwr_countdown_changed = NULL;
		com_drv.fcn_scriptstate_changed = NULL;
		com_drv.fcn_scriptcommand_changed = NULL;
		// com_drv.fcn_wlan_client_connected = NULL;
	}
}

// read MSState and WLAN password from EEPROM
void Init_EEPROM()
{
#ifdef DEBUG_MESSAGES_ACTIVE
	Serial_Printing_Port.println("\nInitializing EEPROM\n");
#endif

	EEPROM.begin(EEPROM_SIZE);

	// ms_available = readEEPROM(0);

	// if (ms_available == 0xFF)
	// 	writeEEPROM(false, 0);
}

// read wlan password and write it to the EEPROM
void update_WLAN_password(uint32_t password)
{
	char string_pw[10];
	char new_pw[10];
	// convert uint32_t password into char
	sprintf(string_pw, "%d", password);
	sprintf(new_pw, "%08s", string_pw); // padding zeros if the chosen password has leading 0s
	update_AP_password((const char *)new_pw);
}

// write one byte into EEPROM (more bytes at once are not possible)
void writeEEPROM(uint8_t data, uint8_t addr)
{
	EEPROM.begin(EEPROM_SIZE);
	EEPROM.write(addr, data);
	EEPROM.end();
}

// write uint32_t to EEPROM
void writeEEPROM_u32(uint32_t data, uint8_t addr)
{
	EEPROM.begin(EEPROM_SIZE);
	uint8_t temp[4];
	memcpy(temp, &data, sizeof(uint32_t));
	for (uint8_t i = 0; i < 4; ++i)
	{
		writeEEPROM(temp[i], i + addr);
	}
	EEPROM.end();
}

// read one byte from EEPROM
uint8_t readEEPROM(uint8_t addr)
{
	uint8_t data;
	EEPROM.begin(EEPROM_SIZE); // already called during Init_MSState
	data = EEPROM.read(addr);
	EEPROM.end();
	return data;
}

// read uint32_t from EEPROM
uint32_t readEEPROM_u32(uint8_t addr)
{
	EEPROM.begin(EEPROM_SIZE);
	uint8_t temp[4];
	uint32_t ret;
	for (uint8_t i = 0; i < 4; ++i)
	{
		temp[i] = EEPROM.read(i + addr);
	}
	memcpy(&ret, &temp[0], sizeof(uint32_t));
	EEPROM.end();

	return ret;
}

// return Baudrate
Baudrate_e get_baudrate(uint32_t baud)
{
	Baudrate_e ret = Baud_NA;

	switch (baud)
	{
	case 300:
		ret = Baud_300;
		break;
	case 600:
		ret = Baud_600;
		break;
	case 1200:
		ret = Baud_1200;
		break;
	case 2400:
		ret = Baud_2400;
		break;
	case 4800:
		ret = Baud_4800;
		break;
	case 9600:
		ret = Baud_9600;
		break;
	case 14400:
		ret = Baud_14400;
		break;
	case 19200:
		ret = Baud_19200;
		break;
	case 38400:
		ret = Baud_38400;
		break;
	case 57600:
		ret = Baud_57600;
		break;
	case 62500:
		ret = Baud_62500;
		break;
	case 115200:
		ret = Baud_115200;
		break;
	default:
		ret = Baud_NA;
		break;
	}

	return ret;
}


/* Wi-Fi functions */

// V: function that will call other wifi functions for processing.
void take_care_of_wifi(void)
{
	Serial_Printing_Port.println("In Wi-Fi taking care function.");
	// V: first read the Wi-Fi string & len and store them in variables.
	eeprom_wifi_len = EEPROM.read(EEPROM_WIFI_LEN_ADDRESS);

	if((eeprom_wifi_len != 0) && (eeprom_wifi_len != 0xff))
	{
		// if length is store then read the Wi-Fi string.
		for( uint8_t i = 0; i<eeprom_wifi_len; i++)
		{
			eeprom_wifi[i] = eeprom_read_wifi_str_char(i + EEPROM_WIFI_STR_ADDRESS);
		}
		eeprom_wifi[eeprom_wifi_len] = '\0';

		uint8_t length = find_wifi_str_len(eeprom_wifi);
		if(length == eeprom_wifi_len)
		{
			Wifi_string = eeprom_wifi;
			Wifi_is_stored = true;
		}
		else
		{
			Serial_Printing_Port.println("EEPROM read data did not matched.");
		}
	}
	else // if length itself is not there then nothing is stored then intialise Wi-Fi with default name.
	{
		Serial_Printing_Port.println("Wi-Fi string is not stored so default Wi-Fi name will be initialised.");
		Wifi_is_stored = false;
		Wifi_string = default_wifi_str;
	}

	Serial_Printing_Port.print("Finalised Wi-Fi name is : ");
	Serial_Printing_Port.println(Wifi_string);
}

// V: function that will find the length of Wi-Fi string.
int find_wifi_str_len(char *str)
{
	int len = 0, i=0;

	while(str[i])
	{
		len++;
		i++;
	}

	return len;
}

// V: function that will write the Wi-Fi string and length of Wi-Fi string into respective addresses
void eeprom_write_wifi_str_char(char data, uint8_t addr)
{
	EEPROM.write(addr, data);
	EEPROM.commit();
}


// V: function that will write the Wi-Fi string and length of Wi-Fi string into respective addresses
char eeprom_read_wifi_str_char(uint8_t addr)
{
	 char data = '\0';
	 data = EEPROM.read(addr);
	 return data;
}


// V: function that will compare the received Wi-Fi string & and already existing Wi-Fi string in EEPROM.
bool compare_wifi_strings(char *eeprom_wifi_str, char *new_wifi_str, int len)
{
	for(int  i=0; i<len; i++)
	{
		if(eeprom_wifi_str[i] != new_wifi_str[i])
		{
			return false;
		}
	}

	return true;
}


// V: function that will erase the data stored in the EEPROM.
void erase_eeprom_data(void)
{
	EEPROM.begin(EEPROM_SIZE);

	int len = EEPROM.read(EEPROM_WIFI_LEN_ADDRESS);

	EEPROM.write(EEPROM_WIFI_LEN_ADDRESS, 0);

	for(uint8_t i=0; i<len; i++)
	{
		EEPROM.write(EEPROM_WIFI_STR_ADDRESS + i, 0);
	}

	EEPROM.commit();
	EEPROM.end();
}




void Arduino_OTA_functionality(void)
{
	// OTA related code
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("ETS");

  Serial_Printing_Port.println("\r\nOTA Functionality started.");

  ArduinoOTA.onStart([]()
  {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
	  Serial_Printing_Port.println("\nFiletype is Firmware.");
    }
    else
    {  // U_FS
      type = "filesystem";
	  Serial_Printing_Port.println("\nFiletype is SPIFFS.");
    }
      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial_Printing_Port.println("ETS Start updating " + type);
  });

  ArduinoOTA.onEnd([]()
  {
    Serial_Printing_Port.println("\nETS Update End");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    Serial_Printing_Port.printf("ETS Update Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial_Printing_Port.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial_Printing_Port.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial_Printing_Port.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial_Printing_Port.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial_Printing_Port.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial_Printing_Port.println("End Failed");
    }
  });

  ArduinoOTA.begin();
  Serial_Printing_Port.println("ETS OTA Ready");
}

bool flag_one = true;

// File Upload Handler
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) 
{
	if(flag_one)
	{
		Serial_Printing_Port.println("\n\nUpload Started");
		Serial_Printing_Port.printf("Filename: %s\n", filename.c_str());
		Serial_Printing_Port.printf("Final: %s\n", final ? "Yes" : "No");
	}
	// yield();
	ESP.wdtFeed();
	delay(24);
	ESP.wdtFeed();
	String updateType;
	

if (!index)
{
    // First chunk of upload
    updateType = request->arg("updatetype");
    if (updateType == "firmware") 
	{
		if(flag_one)
		{
			Serial_Printing_Port.println("User Choose the firmware update.");
		}
		// Prepare firmware update
		Update.begin(APP_FILE_SIZE, U_FLASH);
	} else if (updateType == "filesystem") 
	{
		if(flag_one)
		{
			Serial_Printing_Port.println("User Choose the filesystem update.");
		}
		// Prepare SPIFFS update
		SPIFFS.end();
		//   size_t availableSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
		if (!Update.begin(WEB_FILE_SIZE, U_FS))
		{
			Serial_Printing_Port.println("Cannot begin SPIFFS update");
		}
	}
	else
	{
		Serial_Printing_Port.println("Invalid update type.");
		request->send(400, "text/plain", "Invalid update type.");
		return;
	}
	flag_one = false;
  }

  Serial_Printing_Port.printf("\n\nIndex: %d\n", index);
  Serial_Printing_Port.printf("Length: %d\n\n", len);

  // Write uploaded content with debugging.
 if (len) 
 {
	if(len > CHUNK_SIZE)
	{
		// Serial_Printing_Port.printf("Chunk Received is %d, So parsing it and writing into memory.\n",len);
		ESP.wdtFeed();
		size_t len2 = len;
		while (len2 >= CHUNK_SIZE)
		{
			ESP.wdtFeed();
			size_t written = Update.write(data, CHUNK_SIZE);
			// Serial_Printing_Port.printf("Writing %d bytes After parsing.\n", CHUNK_SIZE);
			if (written != CHUNK_SIZE)
			{
				Serial_Printing_Port.printf("Write error: only %d of %d bytes written\n", written, CHUNK_SIZE);
				Update.printError(Serial);
				request->send(500, "text/plain", "Write error");
				return;
			}
			data = data + CHUNK_SIZE;
			len2 = len2 - CHUNK_SIZE;
		}

		size_t written = Update.write(data, len2);
			// Serial_Printing_Port.printf("Writing %d bytes these are reamaining bytes after parsing.\n", len2);
			if (written != len2)
			{
				Serial_Printing_Port.printf("Write error: only %d of %d bytes written\n", written, len2);
				Update.printError(Serial);
				request->send(500, "text/plain", "Write error");
				return;
			}
	}
	else
	{
		size_t written = Update.write(data, len);
		// Serial_Printing_Port.printf("Writing %d bytes\n", len);
		if (written != len)
		{
			Serial_Printing_Port.printf("Write error: only %d of %d bytes written\n", written, len);
			Update.printError(Serial);
			request->send(500, "text/plain", "Write error");
			return;
		}
	}
  }
  ESP.wdtFeed();

  if (final) {
    // Finalize update
    if (Update.end(true)) {
      Serial_Printing_Port.printf("Update Success: %u bytes\nRebooting...\n", len);
      request->send(200, "text/plain", "Update complete. Rebooting...");
      delay(100);
      ESP.restart();
    } else {
      Update.printError(Serial_Printing_Port);
      request->send(500, "text/plain", "Update failed");
    }
  }
  ESP.wdtFeed();
}

void FOTA_Through_Webpage(void)
{
	// Serve the Webpage containing OTA upload form
	server.on("/updateOTA", HTTP_GET, [](AsyncWebServerRequest *request)
			  { 
				Serial_Printing_Port.println("\nServing update Webpage\n");
				request->send(SPIFFS, "/update.html", String(), false); });

	// Handle Firmware and Filesystem OTA Updates
	server.on("/update", HTTP_POST, 
    [](AsyncWebServerRequest *request) {
      // Response handler
      if (Update.hasError()) {
        request->send(500, "text/plain", "UPDATE FAILED");
      } else {
        request->send(200, "text/plain", "Update complete. Rebooting...");
        delay(1000);
        ESP.restart();
      }
    }, 
    handleFileUpload
  );
}





void Print_system_info(void)
{
	FSInfo fs_info;
	SPIFFS.info(fs_info);

	float fileTotalKB = (float)fs_info.totalBytes / 1024.0;
	float fileUsedKB = (float)fs_info.usedBytes / 1024.0;

	float flashChipSize = (float)ESP.getFlashChipSize() / 1024.0 / 1024.0;
	float realFlashChipSize = (float)ESP.getFlashChipRealSize() / 1024.0 / 1024.0;
	float flashFreq = (float)ESP.getFlashChipSpeed() / 1000.0 / 1000.0;
	FlashMode_t ideMode = ESP.getFlashChipMode();

	Serial_Printing_Port.printf("\n\n#####################\n");

	Serial_Printing_Port.printf("__________________________\n\n");
	Serial_Printing_Port.println("Firmware: ");
	Serial_Printing_Port.printf("    Chip Id: %08X\n", ESP.getChipId());
	Serial_Printing_Port.print("    Core version: ");
	Serial_Printing_Port.println(ESP.getCoreVersion());
	Serial_Printing_Port.print("    SDK version: ");
	Serial_Printing_Port.println(ESP.getSdkVersion());
	Serial_Printing_Port.print("    Boot version: ");
	Serial_Printing_Port.println(ESP.getBootVersion());
	Serial_Printing_Port.print("    Boot mode: ");
	Serial_Printing_Port.println(ESP.getBootMode());

	Serial_Printing_Port.printf("__________________________\n\n");

	Serial_Printing_Port.println("Flash chip information: ");
	Serial_Printing_Port.printf("    Flash chip Id: %08X (for example: Id=001640E0  Manuf=E0, Device=4016 (swap bytes))\n", ESP.getFlashChipId());
	Serial_Printing_Port.print(flashChipSize);
	Serial_Printing_Port.printf("    Sketch thinks Flash RAM is size: ");
	Serial_Printing_Port.println(" MB");
	Serial_Printing_Port.print("    Actual size based on chip Id: ");
	Serial_Printing_Port.print(realFlashChipSize);
	Serial_Printing_Port.println(" MB ... given by (2^( \"Device\" - 1) / 8 / 1024");
	Serial_Printing_Port.print("    Flash frequency: ");
	Serial_Printing_Port.print(flashFreq);
	Serial_Printing_Port.println(" MHz");
	Serial_Printing_Port.printf("    Flash write mode: %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT"
																		 : ideMode == FM_DIO	? "DIO"
																		 : ideMode == FM_DOUT	? "DOUT"
																								: "UNKNOWN"));

	Serial_Printing_Port.printf("__________________________\n\n");

	Serial_Printing_Port.println("File system (SPIFFS): ");
	Serial_Printing_Port.print("    Total KB: ");
	Serial_Printing_Port.print(fileTotalKB);
	Serial_Printing_Port.println(" KB");
	Serial_Printing_Port.print("    Used KB: ");
	Serial_Printing_Port.print(fileUsedKB);
	Serial_Printing_Port.println(" KB");
	Serial_Printing_Port.printf("    Block size: %lu\n", fs_info.blockSize);
	Serial_Printing_Port.printf("    Page size: %lu\n", fs_info.pageSize);
	Serial_Printing_Port.printf("    Maximum open files: %lu\n", fs_info.maxOpenFiles);
	Serial_Printing_Port.printf("    Maximum path length: %lu\n\n", fs_info.maxPathLength);

	Dir dir = SPIFFS.openDir("/");
	Serial_Printing_Port.println("SPIFFS directory {/} :");
	while (dir.next())
	{
		Serial_Printing_Port.print("  ");
		Serial_Printing_Port.println(dir.fileName());
	}

	Serial_Printing_Port.printf("__________________________\n\n");

	Serial_Printing_Port.printf("CPU frequency: %u MHz\n\n", ESP.getCpuFreqMHz());
	Serial_Printing_Port.print("\n#####################\n");
}

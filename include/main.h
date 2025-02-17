/***********************************************************
 *
 * main.h
 * SYSTART GmbH
 * MIZ, 30.06.2021
 *
 ***********************************************************/

#ifndef MAIN_H_
#define MAIN_H_

/***********************************************************
 * Includes
 ***********************************************************/
#include "a_com.h"

/***********************************************************
 * Defines
 ***********************************************************/
#define DEBUG_MESSAGES_ACTIVE
#define WIFI_MODE_AP	 // V: Through this MACRO Wi-Fi Module is used as Access Point
#define IS_ESP_DEV_BOARD // V: uncomment it if you are using a ESP-DEV-BOARD

#define Off 0xFFFFFFFFUL
#define uOff 0xFF
#define OVP_TOT_MAX(x) (x * 1.2)
// #define EEPROM_SIZE 5 // define used EEPROM size (0-512)
#define EEPROM_SIZE 40  		// V: declared newly for storing wi-fi string related data. (0-512)
#define EEPROM_WIFI_LEN_ADDRESS 7		// V: In this Address Wi-Fi String lengtht will be stored.
#define EEPROM_WIFI_STR_ADDRESS 8 		// V: In this Address Actual Wi-Fi string will be stored.

#define WLAN_VERSION 003

/***********************************************************
 * Typ Definitionen   V: Typedef definitions
 ***********************************************************/

typedef struct
{
	float U;
	float I;
	float P;
	float R;
} NtMonitor_t; // V: This Struct will contain V, I, P, R Values

typedef enum
{
	MS_Off = 0,
	MS_Serial,
	MS_Parallel,
	MS_Independent,
} MasterSlaveMode_e;

typedef struct
{
	// bool MS0;
	// bool MS1;
	// bool MS2;
	// bool MS3;
	// bool MS4;
	// bool MS5;
	// bool MS6;
	// bool MS7;
	// bool MS8;
	// bool MS9;
	// bool MS10;
	// bool MS11;
	// bool MS12;
	// bool MS13;
	// bool MS14;
	// bool MS15;
	uint16_t MS0 : 1;
	uint16_t MS1 : 1;
	uint16_t MS2 : 1;
	uint16_t MS3 : 1;
	uint16_t MS4 : 1;
	uint16_t MS5 : 1;
	uint16_t MS6 : 1;
	uint16_t MS7 : 1;
	uint16_t MS8 : 1;
	uint16_t MS9 : 1;
	uint16_t MS10 : 1;
	uint16_t MS11 : 1;
	uint16_t MS12 : 1;
	uint16_t MS13 : 1;
	uint16_t MS14 : 1;
	uint16_t MS15 : 1;
} MSPartner_e;

typedef struct
{
	uint8_t ms_mode; // MasterSlaveMode_e
	uint8_t Nr;		 // Eigene ID-Nr.
	MSPartner_e ms_net_info;

} MasterSlave_t;

typedef struct
{
	float Umax;			  // Maximale Ausgangsspannung [V] 				V: Maximum Output Voltage
	float Imax;			  // Maximaler Ausgangsstro m [A]					V: Maximum Output Current [A]
	float Pmax;			  // Maximale Ger�teleistung [W]				   V: Maximum Device Power [W]
	float Rimax;		  // Maximale Ausgangswiderstand RI Mode[Ohm]    V: Maximum Output Resistance RI Mode [Ohm]
	float Rimin;		  // Minimaler Ausgangswiderstand RI Mode[Ohm]   V: Minimum Output Resistance RI Mode [Ohm]
	float TolU;			  // Genauigkeit Spannung							V: Voltage Accuracy
	float TolI;			  // Genauigkeit Stro m							V: Current Accuracy
	float TolP;			  // Genauigkeit Leistung							V: Power Accuracy
	float TolRi;		  // Genauigkeit Ausgangswiderstand RI Mode		V: Output Resistance RI Mode Accuracy
	bool RS232_available; // RS232 vorhanden						V: RS232 available
	bool RS485_available; // RS485 vorhanden						V: RS485 available
	bool LAN_available;	  // LAN Modul vorhanden					V: LAN Module available
	bool USB_available;	  // R�ckseitige USB vorhanden			   V: Rear USB available
	bool CAN_available;	  // CAN vorhanden							V: CAN available
	bool GPIB_available;  // GPIB vorhanden						V: GPIB available
	bool MS_available;	  // M/S vorhanden							V: MS available
	bool AI_available;	  // AI vorhanden							V: AI available
	bool Uneg;			  // Spannung kann positiv und negativ sein			V: Voltage can be positive and negative
	bool Ineg;			  // Strom kann positiv und negativ sein			V: Current can be positive and negative

} DeviceParam_t;

typedef struct
{
	float MaxU;
	float MaxI;
	float MinU;
	float MinI;
	uint8_t Kennlinie[256];
} Kennlinie_t;

typedef enum
{
	ErrMsg_NoError = 0,
	ErrMsg_NonPermanent,
	ErrMsg_Permanent,
} ErrMsgType_e;

typedef struct
{
	ErrMsgType_e errortype;
	uint8_t msg[78];
} ErrMessage_t;

// interfaces parameters
typedef enum
{
	Baud_NA = 0,
	Baud_300,
	Baud_600,
	Baud_1200,
	Baud_2400,
	Baud_4800,
	Baud_9600,
	Baud_14400,
	Baud_19200,
	Baud_38400,
	Baud_57600,
	Baud_62500,
	Baud_115200
} Baudrate_e;

typedef enum
{
	Parity_None = 0,
	Parity_Even,
	Parity_Odd
} Parity_e;

typedef enum
{
	Databits_7 = 0,
	Databits_8,
} Databits_e;

typedef enum
{
	Stopbits_1 = 0,
	Stopbits_2,
} Stopbits_e;

typedef enum
{
	Handshake_None = 0,
	Handshake_HW,
	Handshake_SW
} Handshake_e;

typedef enum
{
	Echo_Off = 0,
	Echo_On
} Echo_e;

typedef enum
{
	IfType_NA = 0,
	IfType_RS232,
	IfType_RS485,
	IfType_USB,
	IfType_GPIB,
	IfType_LAN,
} IfType_e;

typedef struct
{
	IfType_e type;
	Baudrate_e baudrate;
	Parity_e parity;
	Databits_e databits;
	Stopbits_e stopbits;
	Handshake_e handshake;
	Echo_e echo;
	uint8_t TO;
	uint8_t Addr;
} Iface_t;

typedef struct
{
	bool enabled;
	uint32_t password;
} WLAN_t;

typedef enum
{
	Front_Source = 0,
	WLAN_Source = 1,
} Local_Source_e;

typedef struct
{
	// IDString_t compid;
	ErrMessage_t errmsg;
	// Protection_t protection;
	DeviceParam_t devparams;
	// Config_t config;
	// PIDCtrl_t * pidcontroller;
	Iface_t interface[16];
	// IfaceAI_t interfaceai;
	// Ident_t identity;
	NtMonitor_t monitor;
	// Setvalue_t setvalue;
	// Status_t status;
	MasterSlave_t msconfig;
	NtMonitor_t msmonitor[16];
	// Admin_t admin;  // replcement for: uint8_t	touchActive;
	WLAN_t wlanconfig;
	Kennlinie_t kennlinie;
	// scriptmode_state_e scriptstate;
} Netzteil_Parameters_t;

/***********************************************************
 * Exported Variables
 ***********************************************************/
extern com_drv_t com_drv;
extern Netzteil_Parameters_t NtParam;

/***********************************************************
 * Funktionsdeklarationen
 ************************************************************/
// V: function that will add the Arduino OTA functionality
void Arduino_OTA_functionality(void);

// V: function that will Arduino oTA functionality but allowing update sthrough webpage.
void FOTA_Through_Webpage(void);

#endif /* MAIN_H_ */
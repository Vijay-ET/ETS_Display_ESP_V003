#ifndef APPLICATION_A_COM_H_
#define APPLICATION_A_COM_H_

/***********************************************************
 * Includes
 ***********************************************************/

#include <stdint.h>
#include "d_com.h"


/***********************************************************
 * Defines
 ***********************************************************/

#define QLFY_COUNTER 63
#define SND_PACKET_TIMEOUT 100
#define KOMP_ID_LENGTH 256

/***********************************************************
 * Data types, enums & structs
 ***********************************************************/

typedef enum
{
	REQ_RST = 0,
	REQ_ACK = 1,
	REQ_NACK
} ReqType_e;

typedef struct
{
	uint16_t ntfy_data_recv : 1; // data is changed from netzteil
	uint16_t qlfy_data_snd : 6;	 // data is marked as dirty -> must be send
	uint16_t qlfy_data_req : 6;	 // nack(req) is marked as dirty -> must be send
	uint16_t queue_ins_snd : 1;	 // packet is marked to send (shows that a data-cmd  is in queue and has never been send)
	ReqType_e queue_ins_req : 2; // 1: nack(req) is marked to send; 2: ack ist marked to send (shows that a req-cmd is in queue and has never been send)
} ProtoState_Bitmask_t;

typedef enum
{
	COM_OK,
	COM_ERR, // placeholder
} comdrvErr_e;

typedef enum
{
	THREAD_MAIN,
	THREAD_RX
} sendThread_e;

// Protocoll data types (V10)

typedef struct
{
	uint16_t softwareId;
	uint16_t hardwareId;
} T_Ident;

typedef struct
{
	float umax = 30;			  // max Ausgangsspannung [V]
	float imax = 400;			  // max Ausgangsstrom [A]
	float pmax = 980;			  // max Ausgangsleistung [W]
	float rimax;				  // Maximale Ausgangswiderstand-RI-Mode[Ohm]
	float rimin;				  // Minimaler Ausgangswiderstand-RI-Mode[Ohm]
	float tol_u;				  // Genauigkeit Spannung
	float tol_i;				  // Genauigkeit Strom
	float tol_p;				  // Genauigkeit Leistung
	float tol_ri;				  // Genauigkeit Ausgangswiderstand-RI-Mode
	uint32_t RS232_available : 1; // RS232 present
	uint32_t RS485_available : 1; // RS485 present
	uint32_t LAN_available : 1;	  // LAN Module present
	uint32_t USB_available : 1;	  // rear USB present
	uint32_t CAN_available : 1;	  // CAN present
	uint32_t GPIB_available : 1;  // GPIB present
	uint32_t MS_available : 1;	  // M/S present
	uint32_t AI_available : 1;	  // AI present
	uint32_t Uneg : 1;			  // Voltage can be positive and negative
	uint32_t Ineg : 1;			  // Current can be positive and negative
	uint32_t Language : 4;		  // Language
	uint32_t PolMode : 1;		  // Polarity Mode - Free/Standby-1/0
	uint32_t resetStandby : 1;	  // 1: clear standby at startup, 0: Set standby at startup
	uint32_t _unused : 16;		  // Unused padding
								  //  22:
} T_DevParameter;

typedef struct
{
	char id[KOMP_ID_LENGTH];
} T_Compid;

typedef struct
{
	uint16_t Errortype; // 0 – non error; 1 – non permanent; 2 – permanent
	// V: After change.
	// union {
	// 	uint16_t Errortype; 			/* 0 - non error; 1 - non permanent; 2 - permanent */
	// 	struct {
	// 		uint16_t emsgType :2; 		/* 0- No Message, 1 - non permanent, 2 - permanent */
	// 		uint16_t emsgColour :3; 	/* Color code */
	// 		uint16_t _unused :11;
	// 	};
	// } message_type;
	char Message[78];
} T_ErrMessage;

typedef struct
{
	float P;
	float I;
	float D;
} T_ReglerParam;

typedef struct
{
	uint32_t Typ : 4;		// 0: Keine, 1: RS232, 2: RS485, 3: USB, 4: GPIB, 5: LAN
	uint32_t Datenbits : 1; // 0: 7Bit, 1: 8Bit
	uint32_t Stopbits : 1;	// 0: 1Bit, 1: 2Bit
	uint32_t Parity : 2;	// 0: Keine, 1: Even, 2: Odd
	uint32_t Handshake : 2; // 0: Kein, 1: Hardware, 2: Software
	uint32_t Echo : 1;		// 0: Aus, 1: Ein
	uint32_t _unused_ : 4;
	uint32_t Default : 1; // 1: Als Default speichern
	uint32_t Adresse : 8; //
	uint32_t Timeout : 8; //

	uint32_t baud;
} T_DiParams;

typedef struct
{
	float Ua;		// Ausgangsspannung [V]
	float Ia;		// Ausgangsstrom [A]
	float Pa;		// Ausgangsleistung [W]
	float Unetz[3]; // Phasenspannungen [V] // V: Phase Voltages [V]
	float Inetz[3]; // Netzstroeme [A]      // V: Network Currents [A]
} T_Messdaten;

// typedef struct{
//   uint16_t mode_ms : 2; //bitfield
//   //uint16_t nr_ms : 4;  //bisfield
//   //uint16_t partner: 4; //bitfield
// }T_Status_MS;

// typedef struct{
//   // Diese Bits sind Status- und Setzbits
//   uint16_t Standby : 1; // 0: Gerät aktiv, 1: Gerät in Standby
//   uint16_t Mode : 3; // 0: UI-Mode, 1: UIP-Mode, 2: UIR-Mode, 3: Pvsim-Mode, 4: User
//   uint16_t GoToLocal : 1; // Gerät soll in Local-Betrieb wechseln
//   uint16_t AI_Level : 1; // 0: 5V, 1: 10V
//   uint16_t AI_Mode : 1; // 0: UI, 1: UIP
//   uint16_t Interlock_Mode : 1; // 0: Lowaktiv, 1: Highaktiv
//   uint16_t _unused_1 : 8; // padding

//   // Diese Bits sind Status-Bits. Werte, die von der Front gesendet werden,
//   // werden ignoriert.
//   uint16_t Ovp : 1; // Overvoltage protection
//   uint16_t Uvp : 1; // Undervoltage protection
//   uint16_t Ocp : 1; // Overcurrent protection
//   uint16_t LimMode : 2; // 0: CV-Mode, 1: CC-Mode, 2: CP-Mode
//   uint16_t Source : 2; // Quelle der Setzwerte, 0: Front, 1: AI, 2: Dig. Interface
//   uint16_t Interlock : 1; // Interlock aktiv
//   uint16_t LocalLockout : 1; //
// }T_Status;

typedef struct
{
	// These bits are status and set bits
	uint16_t Standby : 1;		 // 0: Device active, 1: device in standby
	uint16_t Mode : 3;			 // 0: UI-Mode, 1: UIP-Mode, 2: UIR-Mode, 3: Pvsim-Mode, 4: User
	uint16_t GoToLocal : 1;		 // The device should change to local mode
	uint16_t Interlock_Mode : 1; // 0: Low active, 1: High active
	uint16_t _unused_1 : 10;	 // padding

	// These bits are status bits. Values sent from the front are ignored.
	uint16_t Ovp : 1;		// Overvoltage protection
	uint16_t Uvp : 1;		// Undervoltage protection
	uint16_t Ocp : 1;		// Overcurrent protection
	uint16_t LimMode : 2;	// 0: CV-Mode, 1: CC-Mode, 2: CP-Mode
	uint16_t Source : 2;	// Source of settings, 0: Front, 1: AI, 2: Dig. Interface
	uint16_t Interlock : 1; // Interlock active
	uint16_t LocalLockout : 1;
	uint16_t Foldback : 2;		 /* 0: OFF 1: Fold CV, 2: Fold CC, 3: Fold CP */
	uint16_t ExternDisable : 1;	 /* 1: Device output(ON/OFF button) is disable by an external signal */
	uint16_t statusOverTemp : 1; /* 1: Device is disable by over temperature */
	uint16_t _unused : 3;
} T_Status;

typedef struct tMsBits // 2 Word
{
	uint16_t Mode : 2;	   // 0: Aus, 1: Serie, 2: Parallel, 3: Unabhängig
	uint16_t Nr : 4;	   // Eigene ID-Nr. hat 4 Bits
	uint16_t _unused : 10; // padding

	uint16_t Partner : 16; // Bitposition 1 signalisiert „Gerät vorhanden“
} T_MSConfig;

typedef struct
{
	char scr1[24];
	char scr2[24];
	char scr3[24];
	char scr4[24];
} T_Script_command;

typedef struct
{
	float MaxU;
	float MaxI;
	float MinU;
	float MinI;
	uint8_t Kennlinie[256];
} T_Kennlinie;

// V: new structs
typedef struct
{
	uint8_t mac_address[6];
	uint8_t ip_address[4];
	uint8_t gateway[4];
	uint16_t IpV6[8]; // IP V6 address (currently unused)
	uint16_t Dhcp : 1;
	uint16_t _unused : 15;
} T_LanInterface;

typedef struct
{
	float Filter;	/* Filter frequency */
	uint16_t Level; /* 0: 10V 1: 5V 2: 3V AI level */
	uint16_t Mode;	/* 0: UI, 1: UIP */
} T_AnalogInterface;

typedef struct
{
	uint16_t Dest;
	uint16_t Version;
} T_UpdateStart;

typedef struct
{
	uint32_t Address;
	uint8_t Data[256]; // Daten
} T_UpdateData;
// ----------------

// typedef enum
// {
// 	Front_Source = 0,
// 	WLAN_Source = 1,
// } Local_Source_e;

typedef struct _com_drv_t
{
	T_Ident Ident;
	T_DevParameter DevParam;
	T_Compid Comp_id;

	T_ErrMessage ErrMessage;

	T_ReglerParam Regler_pm;
	T_ReglerParam Regler_rim;
	T_ReglerParam Regler_pvm;

	T_DiParams Param_di1;
	T_DiParams Param_di2;
	T_DiParams Param_di3;
	T_DiParams Param_di4;
	T_DiParams Param_di5;
	T_DiParams Param_di6;
	T_DiParams Param_di7;
	T_DiParams Param_di8;
	T_DiParams Param_di9;
	T_DiParams Param_di10;
	T_DiParams Param_di11;
	T_DiParams Param_di12;
	T_DiParams Param_di13;
	T_DiParams Param_di14;
	T_DiParams Param_di15;
	T_DiParams Param_di16;

	T_Messdaten Messdaten;
	T_Status Status;
	T_MSConfig MsConfig;
	// T_Status_MS Status_MS;

	T_Messdaten Mess_ms1;
	T_Messdaten Mess_ms2;
	T_Messdaten Mess_ms3;
	T_Messdaten Mess_ms4;
	T_Messdaten Mess_ms5;
	T_Messdaten Mess_ms6;
	T_Messdaten Mess_ms7;
	T_Messdaten Mess_ms8;
	T_Messdaten Mess_ms9;
	T_Messdaten Mess_ms10;
	T_Messdaten Mess_ms11;
	T_Messdaten Mess_ms12;
	T_Messdaten Mess_ms13;
	T_Messdaten Mess_ms14;
	T_Messdaten Mess_ms15;
	T_Messdaten Mess_ms16;

	float U_setz;
	float I_setz;
	float Umpp_setz;
	float Impp_setz;
	float P_setz;
	float Ri_setz;

	float Ovp;
	float Ocp;
	float T_ocp;
	float Uvp;
	float T_uvp;
	float U_slope;
	float I_slope;
	T_AnalogInterface Ai_filter; // float Ai_filter;
	T_LanInterface LAN;
	float U_limit;
	float I_limit;
	float foldbacktm;
	uint32_t polMode; // Polarity mode Free/Standby - 1/0
	uint32_t foldback;
	uint32_t Relase;
	uint32_t oDelay;
	uint32_t tEn;
	uint32_t datalogger;

	uint32_t wlan_pw;

	uint32_t pwrCountdown;

	uint32_t usb_stick;

	uint32_t scriptstate;
	T_Script_command scr_cmd;

	uint32_t scriptrequest;

	uint32_t local_source;
	// uint32_t ws_connect;

	T_Kennlinie Ui_curve;
	T_UpdateStart Update_start;
	T_UpdateData Update_data;
	uint32_t Update_end;

	void (*fcn_ident_changed)(Proto_Num_e num);
	void (*fcn_devpar_changed)(Proto_Num_e num);
	void (*fcn_kompid_changed)(Proto_Num_e num);
	void (*fcn_errmsg_changed)(Proto_Num_e num);
	void (*fcn_regler_changed)(Proto_Num_e num);
	void (*fcn_diparam_changed)(Proto_Num_e num);
	void (*fcn_measdata_changed)(Proto_Num_e num);
	void (*fcn_status_changed)(Proto_Num_e num);
	void (*fcn_msconfig_changed)(Proto_Num_e num);
	void (*fcn_setz_changed)(Proto_Num_e num);
	void (*fcn_internals_changed)(Proto_Num_e num);
	void (*fcn_local_source_changed)(Proto_Num_e num);
	void (*fcn_wlan_password_changed)(Proto_Num_e num);
	void (*fcn_pwr_countdown_changed)(Proto_Num_e num);
	void (*fcn_usb_stick_changed)(Proto_Num_e num);
	void (*fcn_scriptrequest_changed)(Proto_Num_e num);
	// void (*fcn_ws_connect_changed)(Proto_Num_e num);
	void (*fcn_kennlinie_changed)(Proto_Num_e num);
	void (*fcn_update_changed)(Proto_Num_e num);
	void (*fcn_reset_changed)(void);
	void (*fcn_scriptstate_changed)(Proto_Num_e num);
	void (*fcn_scriptcommand_changed)(Proto_Num_e num);
	void (*fcn_send_overflow)(void);
	void (*fcn_recv_error)(int8_t err);
} com_drv_t; // V: This struct will contain every parameter that are presetted in the touch display

/***********************************************************
 * Function prototypes
 ************************************************************/

#endif /* APPLICATION_A_COM_H_ */
/***********************************************************
 *
 * a_ws_com.h
 * SYSTART GmbH
 * MIZ, 30.06.2021
 *
 ***********************************************************/

#ifndef A_WS_COM_H_
#define A_WS_COM_H_

/***********************************************************
 * Includes
 ***********************************************************/
#include <ESPAsyncWebServer.h>
#include "main.h"
#include "a_graph.h"
#include "d_timer.h"

/***********************************************************
 * Defines
 ***********************************************************/

#define MSG_PRIO_LENGTH 128 // needs to be higher than ANZ_MSG (PARA_04.07.2022)
#define MSG_PRIO_LENGTH_MASK (MSG_PRIO_LENGTH - 1)

/***********************************************************
 * Typ Definitionen
 ***********************************************************/

/* !!Wichtig!! enums wie im .js lassen */ // V: /* !!Important!! enums like in .js remain */
typedef enum
{
	NONE = 0,
	MONVAL = 1,
	SET_V = 2,
	SET_I = 3,
	SET_P = 4,
	SET_R = 5,
	SET_VMPP = 6,
	SET_IMPP = 7,
	SET_OVP = 8,
	SET_UVP = 9,
	SET_UVPT = 10,
	SET_OCP = 11,
	SET_OCPT = 12,
	SET_V_SL = 13,
	SET_I_SL = 14,
	SET_VLIM = 15,
	SET_ILIM = 16,
	NT_STATE = 17,
	CONTROL_P = 18,
	CONTROL_RI = 19,
	CONTROL_PV = 20,
	XY_AXIS = 21,
	XY_MPOS = 22,
	XY_PDATA = 23,
	MS_SETTINGS = 24,
	MS_TBL_0 = 25,
	MS_TBL_1 = 26,
	MS_TBL_2 = 27,
	MS_TBL_3 = 28,
	MS_TBL_4 = 29,
	MS_TBL_5 = 30,
	MS_TBL_6 = 31,
	MS_TBL_7 = 32,
	MS_TBL_8 = 33,
	MS_TBL_9 = 34,
	MS_TBL_10 = 35,
	MS_TBL_11 = 36,
	MS_TBL_12 = 37,
	MS_TBL_13 = 38,
	MS_TBL_14 = 39,
	MS_TBL_15 = 40,
	MS_STATE = 41,
	MS_AVAILABLE = 42, // Parameter ob MS-Menü aktiv
	DEV_PARAM = 43,
	SW_ID = 44,
	CompID = 45,
	REMEMBER = 46,
	TDELAY = 47,
	TENABLE = 48,
	LOGGER = 49,
	ERR_MSG = 50,
	DI_PARAM1 = 51,
	DI_PARAM2 = 52,
	DI_PARAM3 = 53,
	DI_PARAM4 = 54,
	DI_PARAM5 = 55,
	DI_PARAM6 = 56,
	DI_PARAM7 = 57,
	DI_PARAM8 = 58,
	DI_PARAM9 = 59,
	DI_PARAM10 = 60,
	DI_PARAM11 = 61,
	DI_PARAM12 = 62,
	DI_PARAM13 = 63,
	DI_PARAM14 = 64,
	DI_PARAM15 = 65,
	DI_PARAM16 = 66,
	COUNTDOWN_PWR = 67,
	USB_ACTIVE = 68,
	STATE_SCRIPT = 69,
	COMMAND_SCRIPT = 70,
	USB_ERR = 71,
	REQUEST_SCRIPT = 72,
	COUNTER = 74,
	SOURCE_LOCAL = 73,
	COUNTER_ACTIVE = 75,
	ALL_DATA = 76,
	KP_ALIVE = 77,
	ANZ_MSG = 78,
} Messages_e; // V: 4 Bytes

typedef enum
{
	ON = 0,
	OFF = 1,
	OFF_PEND = 2,
	ON_PEND = 3
} Power_e; // V: 4 Bytes

typedef enum
{
	VI = 0,
	VIP = 1,
	VIR = 2,
	PVSIM = 3,
	USER = 4,
	SCRIPT = 5
} Mode_e; // V: 4 Bytes

typedef enum
{
	FRONT = 0,
	AI = 1,
	DIG_ITF = 2
} Source_e; // V: 4 Bytes

typedef enum
{
	LOW_ACT = 0,
	HIGH_ACT = 1
} Interlock_e; // V: 4 Bytes

typedef enum
{
	CV = 0,
	CC = 1,
	CP = 2
} LimMode_e; // V: 4 Bytes

typedef enum
{
	EN = 0,
	DE = 1,
	FR = 2,
	ZH = 3,
	JA = 4,
	KO = 5
} Language_e; // V: 4 Bytes

typedef struct
{
	uint8_t digitAfterPointV;
	uint8_t digitAfterPointI;
	uint8_t digitAfterPointP;
	uint8_t digitAfterPointR;
} Accuracy_t; // V: 4 Bytes

typedef struct
{
	Accuracy_t accuracy; // 4b
	uint8_t ausstattung; // 1b
	uint8_t language;	 // 1b + 2b padding //Language_e
	float Umax;
	float Imax;
	float Pmax;
	float Rmax;
	float Rmin;
} DevParam_t; // V: 28 bytes

typedef struct
{ // V: These struct mem values are shown in the home page ---> Monitor Segment
	float v = 0;
	float i = 0;
	float p = 0;
	float r = 0;
} Monitor_t; // V: 16 bytes

typedef struct
{
	uint8_t remember;
	uint8_t Tdelay;
	uint8_t Tenable;
	uint8_t Logger;
} IntParam_t; // 	// V: 4 Bytes

typedef struct
{			 // V: These struct mem values are shown in many places each one will be commented with its place
	float v; // V: all v, i, p, r, vmpp, impp are shown in the prest segment in the home page in their respective modes
	float i;
	float p;
	float r = 0;
	float vmpp = 0;
	float impp = 0;
	float ovp = 0; // V: all ovp, uvp, uvpT, ocp, ocpT are there in setup ---> Protection
	float uvp = 0;
	float uvpT = 0;
	float ocp = 0;
	float ocpT = 0;
	float vSlope = 0; // V: al vSlope, iSlope, vLim, iLim are there in setup ---> config
	float iSlope = 0;
	float vLim = 0;
	float iLim = 0;
} Preset_t; // V: 60 Bytes

// wrState_t Power; // output enable (~Standby)
// 	Mode_e Mode;   // operational mode
// 	bool ReqToLocal; // GoToLocal (switch to local request)

// 	// Readonly bits (these bits are status bits, the display does not change them - they are ignored by the netzteil )
// 	bool Ovp;
// 	bool Uvp;
// 	bool Ocp;
// 	LimMode_e LimMode;
// 	Source_e Source;
// 	bool Interlock;
// 	bool LocalLockout;
// 	uint8_t Foldback;
// 	bool ExternDisable;

// Enum for fault detection
typedef enum
{
	NA = -1,
	Ovp = 0,
	Ocp = 1,
	Uvp = 2,
	Otp = 3
} Fault_e;

// Enum messages to know the status of Websocket
typedef enum
{
	NI = -1,
	DEAD = 0,
	ALIVE = 1,
	NO_DATA = 2,
	CLOSE_WS = 3
} Websocket_com_e;

typedef struct
{
	uint8_t power = OFF;			 // Power_e
	uint8_t mode = VI;				 // Mode_e
	uint8_t ReqToLocal = false;		 // switch to local request
	uint8_t interlockMode = LOW_ACT; // Interlock_e
	uint8_t limMode = CV;			 // LimMode_e
	uint8_t source = FRONT;			 // Source_e
	uint8_t interlock = false;
	uint8_t LocalLockout = false;
	uint8_t fault = NA;
} Status_t; // V: 9 Bytes

typedef struct
{				 // V: I think one Variable will be created of this type for every mem of control_t Struct
	float p = 1; // V: They are inter linked in setup tab of the Touch display and web page
	float i = 1; // V: Default values are 1 ( can change to any for debugging)
	float d = 1;
} Regler_t; // V: 12 Bytes

typedef struct
{ // V: These are the parameters that are there in Setup ---> control page
	Regler_t p;
	Regler_t ri;
	Regler_t pv;
} Control_t; // V: 36 Bytes

typedef struct
{
	Point_t xAxisStart; // 4b
	uint16_t xAxisSize; // 2b
	Point_t xTitleLoc;	// 4b

	Point_t yAxisStart; // 4b
	uint16_t yAxisSize; // 2b
	Point_t yTitleLoc;	// 4b
} GraphParam_t;			// 24 bytes

typedef struct
{
	GraphParam_t graphParam;		  // 24b
	Point_t markerPos;				  // 2b
	Point_t plotData[PLOT_DATA_SIZE]; // 2b
	uint16_t plotDataSize;			  // 2b
} XYPlotData_t;						  // 32 bytes

typedef enum
{
	MSOff = 0,
	serial = 1,
	parallel = 2,
	independant = 3
} MS_state_e; // V: 4 Bytes

typedef struct
{
	uint8_t msmode = 0;
} MS_Status_t; // 8 bytes

typedef struct
{
	uint8_t prio[ANZ_MSG];
	uint8_t sendQueue; //  V: Single row position
	uint8_t toSend;	   //  V: Transmission position
} MSGprio_t;		   // 4 bytes

typedef struct
{						 // V: Setup ---> Factory reset ---> Software Version
	uint16_t softwareId; // V: Created by me to update the software version in the web page(default)
} Ident_t;				 // 2 bytes

typedef struct
{
	char id[KOMP_ID_LENGTH];
} Compid_t; // 256 bytes

typedef enum
{
	SCRIPTSTATE_HALT, // kein script aufgeladen
	SCRIPTSTATE_LOAD, // script is loaded and ready to run
	SCRIPTSTATE_RUN,  // script is running
} scriptmode_state_e; // 4 bytes

typedef struct
{
	char scr_cmd1[24];
	char scr_cmd2[24];
	char scr_cmd3[24];
	char scr_cmd4[24];
} Script_command_t; // 4 bytes

typedef enum
{
	REQUEST_USER_MODE,
	REQUEST_SCRIPT_MODE
} ReqMode_e; // 4 bytes

typedef enum
{
	SOURCE_FRONT,
	SOURCE_WLAN
} Source_Local_e; // 4 bytes

// typedef struct {
// 	//int ID = 0;
// 	float v = 0;
// 	float i = 0;
// 	float p = 0;
// }MS_t;

// typedef struct {
// 	MS_t ms1;
// 	MS_t ms2;
// }MSAll_t;

/***********************************************************
 * Exported Variables
 ***********************************************************/
extern DevParam_t devParam;
extern Monitor_t monitor;
extern Preset_t preset;
extern Status_t status;
extern IntParam_t intParam;
extern uint8_t count_pwr; // value of power countdown
extern uint8_t usb_active;
extern uint8_t state_script; // scriptmode_state_e
extern Script_command_t cmd_script;
extern Control_t control;
extern XYPlotData_t xyPlotData;
extern Ident_t ident;
extern Compid_t compid;
extern uint8_t source_local;
extern bool counter_active;

/***********************************************************
 * Funktionsdeklarationen
 ************************************************************/
void Init_Wifi();
void update_AP_password(const char *password);
void init_WS();
void WS_COM_Handler();
// V: function that will get required data from the STM.
void update_All_Data();

/*	V: This function is saving what is the recent changed value for every instance of STM packet
 *	so when control goes to ws_com_handler it will check only for saved mactching condition
 *	and only that data will be written into the websocket(based on the case statements in ws_send_data()).
 */
void queue_WS_MSG(Messages_e message);

void ws_Send_Text(String data); // Nur für Debug-zwecke nutzen
void ws_Send_Data(Messages_e message);
void start_counter();
void Counter_Handler();
void get_All_WS_data();
void merge_strs(char str1[], char str2[]);
// void ws_Send_Data(Messages_e message, uint8_t id);
// void send_Dummy_Data(void);

#endif /* A_WS_COM_H_ */

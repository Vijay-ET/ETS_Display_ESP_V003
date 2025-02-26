/***********************************************************
 * Includes
 ***********************************************************/

#include <string.h>

#include <ESP8266WiFi.h>

#include "a_display_com.h"
#include "nan.h"
#include "d_timer.h"
#include "Serial_prints.h"
// #include "a_ws_com.h"

/***********************************************************
 * Global variables
 ***********************************************************/

extern Protocol_t ProtoTypes[__PROTO_COUNT];

/***********************************************************
 * Module variables
 ***********************************************************/

static com_drv_t *dataStruct; // V: It is asigned with the driver variable that carries the address of com_drv variable that is declared in main file.

static ProtoState_Bitmask_t protostate[__PROTO_COUNT];
static uint8_t cmd_idx = 1; // sequential number of each unsolicited command to be sent, so that it can be sent in the same order in COM_Handler
static uint32_t nanFF = Off;

static Timer_t send_data_timestamp;
static Timer_t send_req_timestamp;

int local_source_cnt = 0;

/***********************************************************
 * Private data types
 ***********************************************************/

/***********************************************************
 * Private function prototypes
 ***********************************************************/

static void send_data_packet(Proto_Num_e num);
static void send_req_packet(Proto_Num_e num);
static void send_ack_packet(Proto_Num_e num);
static void payload_assign_received(Proto_Num_e num, void *payload);
static void payload_prepare_tosend(Proto_Num_e num, void **payload);
/*
all these help functions or callback functions that are used only in this file these are registered in
callback function table(an array that holds the pointers of this function types). These are used to call the same functions in main file(com_drv)
independently using the datastruct variable in this file(finally original functions in main file will be called)
*/
static void on_Ident_Changed(Proto_Num_e protonum);
static void on_Devpar_Changed(Proto_Num_e protonum);
static void on_Kompid_Changed(Proto_Num_e protonum);
static void on_ErrMessage_Changed(Proto_Num_e protonum);
static void on_Regler_Changed(Proto_Num_e protonum);
static void on_Diparam_Changed(Proto_Num_e protonum);
static void on_Measdata_Changed(Proto_Num_e protonum);
static void on_Status_Changed(Proto_Num_e protonum);
static void on_MsConfig_Changed(Proto_Num_e protonum);
static void on_Setz_Changed(Proto_Num_e protonum);
static void on_Internals_Changed(Proto_Num_e protonum);
static void on_WLAN_Password_Changed(Proto_Num_e protonum);
static void on_PWR_Countdown_Changed(Proto_Num_e protonum);
static void on_USB_Stick_Changed(Proto_Num_e protonum);
static void on_Scriptstate_Changed(Proto_Num_e protonum);
static void on_Scriptcommand_Changed(Proto_Num_e protonum);
static void on_Scriptrequest_Changed(Proto_Num_e protonum);
static void on_Local_Source_Changed(Proto_Num_e protonum);
// static void on_WS_Connect_Changed(Proto_Num_e protonum);
static void on_Kennlinie_Changed(Proto_Num_e protonum);

static void on_Send_Overflow(void);
static void on_Recv_Error(COM_Err_e err);
static void on_Reset_Changed(Proto_Num_e protonum);
static void on_Update_Changed(Proto_Num_e protonum);

// V: Data from STM is stored in the com_drv through payload_assign_received()
// V: Now through this callback_table data will be copied to from com_Drv to NtParam
// V: The particular callbacks will be called based on the flags set for the particular packet
static void (*callback_function_table[__PROTO_COUNT])(Proto_Num_e) = {
	// Funktionspointerarray.
	on_Ident_Changed,		  // IDENT
	on_Devpar_Changed,		  // PARAMETER
	on_Kompid_Changed,		  // COMP_ID
	on_ErrMessage_Changed,	  // ERRORMSG
	on_Regler_Changed,		  // REGLER_PM
	on_Regler_Changed,		  // REGLER_RIM
	on_Regler_Changed,		  // REGLER_PVM
	on_Diparam_Changed,		  // PARAM_DI1
	on_Diparam_Changed,		  // PARAM_DI2
	on_Diparam_Changed,		  // PARAM_DI3
	on_Diparam_Changed,		  // PARAM_DI4
	on_Diparam_Changed,		  // PARAM_DI5
	on_Diparam_Changed,		  // PARAM_DI6
	on_Diparam_Changed,		  // PARAM_DI7
	on_Diparam_Changed,		  // PARAM_DI8
	on_Diparam_Changed,		  // PARAM_DI9
	on_Diparam_Changed,		  // PARAM_DI10
	on_Diparam_Changed,		  // PARAM_DI11
	on_Diparam_Changed,		  // PARAM_DI12
	on_Diparam_Changed,		  // PARAM_DI13
	on_Diparam_Changed,		  // PARAM_DI14
	on_Diparam_Changed,		  // PARAM_DI15
	on_Diparam_Changed,		  // PARAM_DI16
	on_Diparam_Changed,		  // LAN_INTERFACE
	on_Measdata_Changed,	  // MESSDATEN
	on_Status_Changed,		  // NT_STATUS
	on_MsConfig_Changed,	  // MSCONFIG
	on_Measdata_Changed,	  // MESS_MS1
	on_Measdata_Changed,	  // MESS_MS2
	on_Measdata_Changed,	  // MESS_MS3
	on_Measdata_Changed,	  // MESS_MS4
	on_Measdata_Changed,	  // MESS_MS5
	on_Measdata_Changed,	  // MESS_MS6
	on_Measdata_Changed,	  // MESS_MS7
	on_Measdata_Changed,	  // MESS_MS8
	on_Measdata_Changed,	  // MESS_MS9
	on_Measdata_Changed,	  // MESS_MS10
	on_Measdata_Changed,	  // MESS_MS11
	on_Measdata_Changed,	  // MESS_MS12
	on_Measdata_Changed,	  // MESS_MS13
	on_Measdata_Changed,	  // MESS_MS14
	on_Measdata_Changed,	  // MESS_MS15
	on_Measdata_Changed,	  // MESS_MS16
	on_Setz_Changed,		  // U_SETZ
	on_Setz_Changed,		  // I-SETZ
	on_Setz_Changed,		  // UMPP_SETZ
	on_Setz_Changed,		  // IMPP_SETZ
	on_Setz_Changed,		  // R_SETZ
	on_Setz_Changed,		  // RI_SETZ
	on_Setz_Changed,		  // OVP
	on_Setz_Changed,		  // OCP
	on_Setz_Changed,		  // T_OCP
	on_Setz_Changed,		  // UVP
	on_Setz_Changed,		  // T_UVP
	on_Setz_Changed,		  // U_SLOPE
	on_Setz_Changed,		  // I_SLOPE
	on_Setz_Changed,		  // ANALOG_INTERFACE
	on_Setz_Changed,		  // U_LIMIT
	on_Setz_Changed,		  // I_LIMIT
	on_Setz_Changed,		  // FOLDBACK
	on_Setz_Changed,		  // FOLDBACKTM
	on_Internals_Changed,	  // REMEMBER_LS
	on_Internals_Changed,	  // ODELAY
	on_Internals_Changed,	  // TEN
	on_Internals_Changed,	  // DATALOGGER
	on_WLAN_Password_Changed, // WLAN_PW
	NULL,					  // PWR_COUNTDWN
	NULL,					  // USB_STICK
	NULL,					  // SCRIPTSTATE
	NULL,					  // SCRIPTCOMMAND
	NULL,					  // SCRIPTREQUEST
	on_Local_Source_Changed,					  // LOCAL_SOURCE
	on_Setz_Changed,		  // POL_MODE
	on_Kennlinie_Changed,	  // UI_CURVE (or) WS_CONNECT
	on_Reset_Changed,		  // SOFT_RESET
	on_Update_Changed,		  // UPDATE_START
	NULL,					  // UPDATE_DATA
	NULL,					  // UPDATE_END (CITT16 checksum)
};

/***********************************************************
 * Public function definitions
 ***********************************************************/

/**
 * Init callback function-pointers with null
 * @param Pointer to com-driver struct
 * @return void
 */
void COM_Driver_Init(com_drv_t *driver)
{

	driver->fcn_ident_changed = NULL;
	driver->fcn_devpar_changed = NULL;
	driver->fcn_kompid_changed = NULL;
	driver->fcn_errmsg_changed = NULL;
	driver->fcn_regler_changed = NULL;
	driver->fcn_diparam_changed = NULL;
	driver->fcn_measdata_changed = NULL;
	driver->fcn_status_changed = NULL;
	driver->fcn_msconfig_changed = NULL;
	driver->fcn_setz_changed = NULL;
	driver->fcn_internals_changed = NULL;
	driver->fcn_local_source_changed = NULL;
	driver->fcn_wlan_password_changed = NULL;
	driver->fcn_pwr_countdown_changed = NULL;
	driver->fcn_usb_stick_changed = NULL;
	driver->fcn_scriptrequest_changed = NULL;
	// driver->fcn_ws_connect_changed = NULL;
	driver->fcn_kennlinie_changed = NULL;
	driver->fcn_update_changed = NULL;
	driver->fcn_reset_changed = NULL;
	driver->fcn_scriptstate_changed = NULL;
	driver->fcn_scriptcommand_changed = NULL;
	driver->fcn_send_overflow = NULL;
	driver->fcn_recv_error = NULL;
}

/**
 * Point local con-driver struct to extern struct
 * @param Pointer to com-driver struct
 * @return Errorstate
 */
comdrvErr_e COM_Driver_Register(com_drv_t *driver)
{
	dataStruct = driver; // V: dataStruct is of type com_drv_t

	// init timestamps w timout value, so that the first package of the program can be sent without delay
	send_data_timestamp = SND_PACKET_TIMEOUT;
	send_req_timestamp = SND_PACKET_TIMEOUT;

	return COM_OK;
}

// void serialEvent() {
//     while (Serial.available()) {
//         D_DataReceived(Serial.read());
//     }
// }
/**
 * Handle received data and send queued packets
 * @param 	void
 * @return 	void
 */
void COM_Handler(void) // V: Function that will Recieve Data from STM32 Through UART
{
	// Handle all received bytes
	while (Serial.available())
	{
		ESP.wdtFeed();
		D_DataReceived(Serial.read()); // V: Read Data From UART( Binary Data)
	} 
	ESP.wdtFeed();

	// Check for parameter changes and call the corresponding function
	for (int protnum = 0; protnum < __PROTO_COUNT; ++protnum)
	{
		if (protostate[protnum].ntfy_data_recv == SET) // V: this flag will be set in D_PacketDataValidReceived() for every valid packet received.
		{
			// reset the notification flag
			protostate[protnum].ntfy_data_recv = RESET;
			// notify the upper instance, firing a proper event
			// V: data from the com_drv will be copied to NtParam through this
			if (callback_function_table[protnum])
			{
				callback_function_table[protnum]((Proto_Num_e)protnum);
			}
		}
	}
	// try to flush the pending command in the queue. One command per function call
	uint8_t cmd_earliest_idx = 255;

	/* Simple queue implementation through cmd_idx */

	// check for pending data/nack to send in 1-8 order
	for (uint8_t num = 0; num < __PROTO_COUNT; ++num)
	{
		if (protostate[num].qlfy_data_snd != RESET && protostate[num].qlfy_data_snd < cmd_earliest_idx)
		{
			cmd_earliest_idx = protostate[num].qlfy_data_snd;
		}

		if (protostate[num].qlfy_data_req != RESET && protostate[num].qlfy_data_req < cmd_earliest_idx)
		{
			cmd_earliest_idx = protostate[num].qlfy_data_req;
		}
	}

	if (cmd_earliest_idx == 255)
	{
		// queue is empty. reset the linear index counter cmd_idx
		cmd_idx = 1;
		return;
	}
	ESP.wdtFeed();
	// send 1 command and leave handler
	for (uint8_t num = 0; num < __PROTO_COUNT; ++num)
	{
		if (cmd_earliest_idx == protostate[num].qlfy_data_snd)
		{
			if (timer_get_time() - send_data_timestamp >= SND_PACKET_TIMEOUT)
			{
				protostate[num].queue_ins_snd = RESET;
				send_data_packet((Proto_Num_e)num);		// V: Finally Calls D_sendPacket function and writes Something into UART
				send_data_timestamp = timer_get_time(); // reset the data send timestamp
			}
			break;
		}
		else if (cmd_earliest_idx == protostate[num].qlfy_data_req)
		{
			if (timer_get_time() - send_req_timestamp >= SND_PACKET_TIMEOUT)
			{
				protostate[num].queue_ins_req = REQ_RST;
				send_req_packet((Proto_Num_e)num);	   // V: Finally Calls D_sendPacket function and writes Something into UART
				send_req_timestamp = timer_get_time(); // reset the req send timestamp
			}
			break;
		}
	}
}

/**
 * Enqueue a packet to be send
 * @param protocoltype - protcol paramter type
 * @return void
 */
void COM_SendParameter(Proto_Num_e num)
{
	// #ifndef IS_ESP_DEV_BOARD   //muss auskommentiert sein sonst funktioniert das TouchTest tool nicht
	// Qualify the command if not yet qualified
	if (protostate[num].qlfy_data_snd == RESET)
	{
		protostate[num].qlfy_data_snd = cmd_idx; // data is requested. set the qualify flag for the coresponding protocol.

		if (cmd_idx++ == QLFY_COUNTER) // increment command counter. In case of overflow the commands in queue
			on_Send_Overflow();
	}
	protostate[num].queue_ins_snd = SET; // mark for sending
	// #endif
}

/**
 * Enqueue a request data packet to be send
 * @param protocoltype - protcol parameter type
 * @return void
 */
void COM_GetParameter(Proto_Num_e num)
{
	// Qualify the command if not yet qualified
	if (protostate[num].qlfy_data_snd == RESET)
	{
		protostate[num].qlfy_data_req = cmd_idx; // data is requested. set the qualify flag for the coresponding protocol.

		if (cmd_idx++ == QLFY_COUNTER) // increment command counter. In case of overflow the commands in queue
			on_Send_Overflow();
	}

	protostate[num].queue_ins_req = REQ_NACK; // mark for sending
}

/**
 * Is called when a packet with data correct received in rx handler
 * @param 1 protocoltype - protcol paramter type
 * @param 2 received data
 * @return void
 */
void D_PacketDataValidReceived(Proto_Num_e num, void *payload) // V: this is called in D_DataReceived() function in CS case
{
	// the requested data is received. reset the request flag
	protostate[num].qlfy_data_req = RESET;

	// set notification flag for the corresponding protocol, that data is received
	protostate[num].ntfy_data_recv = SET;

	// send acknowledge
	protostate[num].queue_ins_req = REQ_RST;
	send_ack_packet(num);

	// aquisite the data from payload stream and assign it to the module variables
	payload_assign_received(num, payload);

	// allow the COM_SendParameter/COM_Handler functions to send a new/next(from queue) data packet
	send_req_timestamp = 0;
}

/**
 * Is called when a packet with data but wrong checksum is received in rx handler
 * @param protocoltype - protcol paramter type
 * @return void
 */
void D_PacketDataCorruptedReceived(Proto_Num_e num)
{
	// Qualify the command if not yet qualified
	if (protostate[num].qlfy_data_req == RESET)
	{
		protostate[num].qlfy_data_req = cmd_idx; // data is requested. set the qualify flag for the coresponding protocol.

		if (cmd_idx++ == QLFY_COUNTER) // increment command counter
			on_Send_Overflow();
	}

	// send request
	protostate[num].queue_ins_req = REQ_RST;
	send_req_packet(num);
	send_req_timestamp = timer_get_time(); // reset the req send timestamp
}

/**
 * Is called when a ACK-packet is received in rx handler
 * @param protocoltype - protcol paramter type
 * @return void
 */
void D_PacketACKReceived(Proto_Num_e num)
{
	// // check for data timeout
	if ((timer_get_time() - send_data_timestamp) < SND_PACKET_TIMEOUT)
	// the ack packets are valid only when they are within the timeout boundary
	{
		if (protostate[num].queue_ins_snd == RESET) // Datenpaket wurde zuvor gesendet
			// data is accepted from recepient. clear the dirty flag, previously marked in nack received or in COM_SendParameter function
			protostate[num].qlfy_data_snd = RESET;
	}

	// allow the COM_SendParameter/COM_Handler functions to send a new/next(from queue) data packet
	send_data_timestamp = 0;
}

/**
 * Is called when a NACK-packet is received in rx handler
 * @param protocoltype - protcol paramter type
 * @return void
 */
void D_PacketNACKReceived(Proto_Num_e num)
{
	// Qualify the command if not yet qualified
	if (protostate[num].qlfy_data_snd == RESET)
	{
		protostate[num].qlfy_data_snd = cmd_idx; // data is requested. set the qualify flag for the coresponding protocol.

		if (cmd_idx++ == QLFY_COUNTER) // increment command counter. In case of overflow the commands in queue
			on_Send_Overflow();
	}

	protostate[num].queue_ins_snd = SET; // Send data packet again in COM_Handler()
}

/**
 * Is called on receive Error (overflow, ..) if RECV_ERROR_MSG_LEVEL_x definded
 * @param protocoltype - protcol paramter type
 * @return void
 */
void D_Error(COM_Err_e err)
{
	on_Recv_Error(err);
}

/***********************************************************
 * Private function definitions
 ***********************************************************/

/**
 * Send ACK packet. Use if data correct received
 * @param protocoltype - protcol paramter type
 * @return void
 */
void send_ack_packet(Proto_Num_e num)
{
	uint16_t payload_len = 0;

	// prepare an ACK answer
	Packet_t sndpacket;
	memset(&sndpacket, 0, sizeof(Packet_t));
	sndpacket.packethdr.packettype = Packet_Type_ACK;
	sndpacket.packethdr.datatype.req = RESET;
	sndpacket.packethdr.datatype.payloadtype = ProtoTypes[num].prototype;

	// send an ACK answer
	D_SendPacket(&sndpacket, payload_len);
}

/**
 * Send Request packet. Use if data corrupted received, or new data requested
 * @param protocoltype - protcol paramter type
 * @return void
 */
void send_req_packet(Proto_Num_e num)
{
	uint16_t payload_len = 0;

	// prepare an NACK answer
	Packet_t sndpacket;
	memset(&sndpacket, 0, sizeof(Packet_t));
	sndpacket.packethdr.packettype = Packet_Type_ACK;
	sndpacket.packethdr.datatype.req = SET; // nack
	sndpacket.packethdr.datatype.payloadtype = ProtoTypes[num].prototype;

	D_SendPacket(&sndpacket, payload_len);
}

/**
 * Send data packet with payload
 * @param protocoltype - protcol paramter type—
 * @return void
 */
void send_data_packet(Proto_Num_e num)
{
	// send DATA
	Packet_t sndpacket;
	memset(&sndpacket, 0, sizeof(Packet_t));
	sndpacket.packethdr.packettype = Packet_Type_DATA;
	sndpacket.packethdr.datatype.req = RESET;
	sndpacket.packethdr.datatype.payloadtype = ProtoTypes[num].prototype;

	// prepare payload
	payload_prepare_tosend(num, &sndpacket.payload);
	D_SendPacket(&sndpacket, ProtoTypes[num].payloadlen);
}

/**
 * payload_assign_received
 * @param in this function is performed the assignement of the payload to the com_drv structure parameters
 * @return return_type
 */
void payload_assign_received(Proto_Num_e num, void *payload) // V: this is called by D_PacketDataValidReceived() function.
{
	Serial_Printing_Port.print("What is received : ");
	Serial_Printing_Port.println(num);
	switch (num)
	{
	case IDENT:
		memcpy(&dataStruct->Ident, payload, sizeof(T_Ident)); // V: &dataStruct->Ident, this will give the address of ident as memcpy needs address
		break;
	case PARAMETER:
		memcpy(&dataStruct->DevParam, payload, sizeof(T_DevParameter));
		break;
	case COMP_ID:
		memcpy(&dataStruct->Comp_id, payload, sizeof(T_Compid));
		break;
	case ERRORMSG:
		memcpy(&dataStruct->ErrMessage, payload, sizeof(T_ErrMessage));
		break;
	case REGLER_PM:
		memcpy(&dataStruct->Regler_pm, payload, sizeof(T_ReglerParam));
		break;
	case REGLER_RIM:
		memcpy(&dataStruct->Regler_rim, payload, sizeof(T_ReglerParam));
		break;
	case REGLER_PVM:
		memcpy(&dataStruct->Regler_pvm, payload, sizeof(T_ReglerParam));
		break;
	case PARAM_DI1:
		memcpy(&dataStruct->Param_di1, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI2:
		memcpy(&dataStruct->Param_di2, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI3:
		memcpy(&dataStruct->Param_di3, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI4:
		memcpy(&dataStruct->Param_di4, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI5:
		memcpy(&dataStruct->Param_di5, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI6:
		memcpy(&dataStruct->Param_di6, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI7:
		memcpy(&dataStruct->Param_di7, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI8:
		memcpy(&dataStruct->Param_di8, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI9:
		memcpy(&dataStruct->Param_di9, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI10:
		memcpy(&dataStruct->Param_di10, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI11:
		memcpy(&dataStruct->Param_di11, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI12:
		memcpy(&dataStruct->Param_di12, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI13:
		memcpy(&dataStruct->Param_di13, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI14:
		memcpy(&dataStruct->Param_di14, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI15:
		memcpy(&dataStruct->Param_di15, payload, sizeof(T_DiParams));
		break;
	case PARAM_DI16:
		memcpy(&dataStruct->Param_di16, payload, sizeof(T_DiParams));
		break;
	case LAN_INTERFACE:
		memcpy(&dataStruct->LAN, payload, sizeof(T_LanInterface));
		break;
	case MESSDATEN:
		memcpy(&dataStruct->Messdaten, payload, sizeof(T_Messdaten));
		break;
	case NT_STATUS:
		memcpy(&dataStruct->Status, payload, sizeof(T_Status));
		break;
	case MSCONFIG:
		memcpy(&dataStruct->MsConfig, payload, sizeof(T_MSConfig));
		break;
	case MESS_MS1:
		memcpy(&dataStruct->Mess_ms1, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS2:
		memcpy(&dataStruct->Mess_ms2, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS3:
		memcpy(&dataStruct->Mess_ms3, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS4:
		memcpy(&dataStruct->Mess_ms4, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS5:
		memcpy(&dataStruct->Mess_ms5, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS6:
		memcpy(&dataStruct->Mess_ms6, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS7:
		memcpy(&dataStruct->Mess_ms7, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS8:
		memcpy(&dataStruct->Mess_ms8, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS9:
		memcpy(&dataStruct->Mess_ms9, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS10:
		memcpy(&dataStruct->Mess_ms10, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS11:
		memcpy(&dataStruct->Mess_ms11, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS12:
		memcpy(&dataStruct->Mess_ms12, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS13:
		memcpy(&dataStruct->Mess_ms13, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS14:
		memcpy(&dataStruct->Mess_ms14, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS15:
		memcpy(&dataStruct->Mess_ms15, payload, sizeof(T_Messdaten));
		break;
	case MESS_MS16:
		memcpy(&dataStruct->Mess_ms16, payload, sizeof(T_Messdaten));
		break;
	case U_SETZ:
		dataStruct->U_setz = *(float *)payload;
		break;
	case I_SETZ:
		dataStruct->I_setz = *(float *)payload;
		break;
	case UMPP_SETZ:
		dataStruct->Umpp_setz = *(float *)payload;
		break;
	case IMPP_SETZ:
		dataStruct->Impp_setz = *(float *)payload;
		break;
	case P_SETZ:
		dataStruct->P_setz = *(float *)payload;
		break;
	case RI_SETZ:
		dataStruct->Ri_setz = *(float *)payload;
		break;
	case OVP:
		dataStruct->Ovp = *(float *)payload;
		break;
	case OCP:
		dataStruct->Ocp = *(float *)payload;
		break;
	case T_OCP:
		dataStruct->T_ocp = *(float *)payload;
		break;
	case UVP:
		dataStruct->Uvp = *(float *)payload;
		break;
	case T_UVP:
		dataStruct->T_uvp = *(float *)payload;
		break;
	case U_SLOPE:
		dataStruct->U_slope = *(float *)payload;
		break;
	case I_SLOPE:
		dataStruct->I_slope = *(float *)payload;
		break;
	case AI_FILTER:
		memcpy(&dataStruct->Ai_filter, payload, sizeof(T_AnalogInterface));
		break;
	case U_LIMIT:
		dataStruct->U_limit = *(float *)payload;
		break;
	case I_LIMIT:
		dataStruct->I_limit = *(float *)payload;
		break;
	case FOLDBACK:
		dataStruct->foldback = *(uint32_t *)payload;
		break;
	case FOLDBACKTM:
		dataStruct->foldbacktm = *(float *)payload;
		break;
	case RELASE:
		dataStruct->Relase = *(uint32_t *)payload;
		break;
	case POL_MODE:
		dataStruct->polMode = *(uint32_t *)payload;
		break;
	case UI_CURVE:
		memcpy(&dataStruct->Ui_curve, payload, sizeof(T_Kennlinie));
		break;
	case ODELAY:
		dataStruct->oDelay = *(uint32_t *)payload;
		break;
	case TEN:
		dataStruct->tEn = *(uint32_t *)payload;
		break;
	case DATALOGGER:
		dataStruct->datalogger = *(uint32_t *)payload;
		break;
	case WLAN_PW:
		dataStruct->wlan_pw = *(uint32_t *)payload;
		break;
	case PWR_COUNTDOWN:
		dataStruct->pwrCountdown = *(uint32_t *)payload;
		break;
	case USB_STICK:
		dataStruct->usb_stick = *(uint32_t *)payload;
		break;
	case SCRIPTSTATE:
		dataStruct->scriptstate = *(uint32_t *)payload;
		break;
	case SCRIPTCOMMAND:
		memcpy(&dataStruct->scr_cmd, payload, sizeof(T_Script_command));
		break;
	case SCRIPTREQUEST:
		dataStruct->scriptrequest = *(uint32_t *)payload;
		break;
	case LOCAL_SOURCE:
		dataStruct->local_source = *(uint32_t *)payload;
		break;
	// case WS_CONNECT:
	//     dataStruct->ws_connect = *(uint32_t *)payload;
	//     break;
	default:
		break;
	}
}

/**
 * payload_prepare_tosend
 * @param in this function is prepared the payload to to send
 * @return return_type
 */
void payload_prepare_tosend(Proto_Num_e num, void **payload)
{
	switch (num)
	{
	case IDENT:
		*payload = &dataStruct->Ident;
		break;
	case PARAMETER:
		*payload = &dataStruct->DevParam;
		break;
	case COMP_ID:
		*payload = &dataStruct->Comp_id;
		break;
	case ERRORMSG:
		*payload = &dataStruct->ErrMessage;
		break;
	case REGLER_PM:
		*payload = &dataStruct->Regler_pm;
		break;
	case REGLER_RIM:
		*payload = &dataStruct->Regler_rim;
		break;
	case REGLER_PVM:
		*payload = &dataStruct->Regler_pvm;
		break;
	case PARAM_DI1:
		*payload = &dataStruct->Param_di1;
		break;
	case PARAM_DI2:
		*payload = &dataStruct->Param_di2;
		break;
	case PARAM_DI3:
		*payload = &dataStruct->Param_di3;
		break;
	case PARAM_DI4:
		*payload = &dataStruct->Param_di4;
		break;
	case PARAM_DI5:
		*payload = &dataStruct->Param_di5;
		break;
	case PARAM_DI6:
		*payload = &dataStruct->Param_di6;
		break;
	case PARAM_DI7:
		*payload = &dataStruct->Param_di7;
		break;
	case PARAM_DI8:
		*payload = &dataStruct->Param_di8;
		break;
	case PARAM_DI9:
		*payload = &dataStruct->Param_di9;
		break;
	case PARAM_DI10:
		*payload = &dataStruct->Param_di10;
		break;
	case PARAM_DI11:
		*payload = &dataStruct->Param_di11;
		break;
	case PARAM_DI12:
		*payload = &dataStruct->Param_di12;
		break;
	case PARAM_DI13:
		*payload = &dataStruct->Param_di13;
		break;
	case PARAM_DI14:
		*payload = &dataStruct->Param_di14;
		break;
	case PARAM_DI15:
		*payload = &dataStruct->Param_di15;
		break;
	case PARAM_DI16:
		*payload = &dataStruct->Param_di16;
		break;
	case LAN_INTERFACE:
		*payload = &dataStruct->LAN;
		break;
	case MESSDATEN:
		*payload = &dataStruct->Messdaten;
		break;
	case NT_STATUS:
		*payload = &dataStruct->Status;
		break;
	case MSCONFIG:
		*payload = &dataStruct->MsConfig;
		break;
	case MESS_MS1:
		*payload = &dataStruct->Mess_ms1;
		break;
	case MESS_MS2:
		*payload = &dataStruct->Mess_ms2;
		break;
	case MESS_MS3:
		*payload = &dataStruct->Mess_ms3;
		break;
	case MESS_MS4:
		*payload = &dataStruct->Mess_ms4;
		break;
	case MESS_MS5:
		*payload = &dataStruct->Mess_ms5;
		break;
	case MESS_MS6:
		*payload = &dataStruct->Mess_ms6;
		break;
	case MESS_MS7:
		*payload = &dataStruct->Mess_ms7;
		break;
	case MESS_MS8:
		*payload = &dataStruct->Mess_ms8;
		break;
	case MESS_MS9:
		*payload = &dataStruct->Mess_ms9;
		break;
	case MESS_MS10:
		*payload = &dataStruct->Mess_ms10;
		break;
	case MESS_MS11:
		*payload = &dataStruct->Mess_ms11;
		break;
	case MESS_MS12:
		*payload = &dataStruct->Mess_ms12;
		break;
	case MESS_MS13:
		*payload = &dataStruct->Mess_ms13;
		break;
	case MESS_MS14:
		*payload = &dataStruct->Mess_ms14;
		break;
	case MESS_MS15:
		*payload = &dataStruct->Mess_ms15;
		break;
	case MESS_MS16:
		*payload = &dataStruct->Mess_ms16;
		break;
	case U_SETZ:
		*payload = &dataStruct->U_setz;
		break;
	case I_SETZ:
		*payload = &dataStruct->I_setz;
		break;
	case UMPP_SETZ:
		*payload = &dataStruct->Umpp_setz;
		break;
	case IMPP_SETZ:
		*payload = &dataStruct->Impp_setz;
		break;
	case P_SETZ:
		*payload = &dataStruct->P_setz;
		break;
	case RI_SETZ:
		*payload = &dataStruct->Ri_setz;
		break;
	case OVP:
		*payload = &dataStruct->Ovp;
		break;
	case OCP:
		if (dataStruct->Ocp == Off)
			*payload = &nanFF;
		else
			*payload = &dataStruct->Ocp;
		break;
	case T_OCP:
		*payload = &dataStruct->T_ocp;
		break;
	case UVP:
		if (dataStruct->Uvp == Off)
			*payload = &nanFF;
		else
			*payload = &dataStruct->Uvp;
		break;
	case T_UVP:
		*payload = &dataStruct->T_uvp;
		break;
	case U_SLOPE:
		if (dataStruct->U_slope == Off)
			*payload = &nanFF;
		else
			*payload = &dataStruct->U_slope;
		break;
	case I_SLOPE:
		if (dataStruct->I_slope == Off)
			*payload = &nanFF;
		else
			*payload = &dataStruct->I_slope;
		break;
	case AI_FILTER:
		*payload = &dataStruct->Ai_filter;
		break;
	case U_LIMIT:
		*payload = &dataStruct->U_limit;
		break;
	case I_LIMIT:
		*payload = &dataStruct->I_limit;
		break;
	case FOLDBACK:
		*payload = &dataStruct->foldback;
		break;
	case FOLDBACKTM:
		*payload = &dataStruct->foldbacktm;
		break;
	case RELASE:
		*payload = &dataStruct->Relase;
		break;
	case UI_CURVE:
		*payload = &dataStruct->Ui_curve;
		break;
	case ODELAY:
		*payload = &dataStruct->oDelay;
		break;
	case TEN:
		*payload = &dataStruct->tEn;
		break;
	case DATALOGGER:
		*payload = &dataStruct->datalogger;
		break;
	case WLAN_PW:
		*payload = &dataStruct->wlan_pw;
		break;
	case PWR_COUNTDOWN:
		*payload = &dataStruct->pwrCountdown;
		break;
	case USB_STICK:
		*payload = &dataStruct->usb_stick;
		break;
	case SCRIPTSTATE:
		*payload = &dataStruct->scriptstate;
		break;
	case SCRIPTCOMMAND:
		*payload = &dataStruct->scr_cmd;
		break;
	case SCRIPTREQUEST:
		*payload = &dataStruct->scriptrequest;
		break;
	case LOCAL_SOURCE:
		*payload = &dataStruct->local_source;
		break;
	// case WS_CONNECT:
	// 	*payload = &dataStruct->ws_connect;
	// 	break;

	default:
		break;
	}
}

/***********************************************************
 * Callback Functions
 ***********************************************************/
void on_Ident_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_ident_changed != NULL)
		dataStruct->fcn_ident_changed(protonum);
}

void on_Devpar_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_devpar_changed != NULL)
		dataStruct->fcn_devpar_changed(protonum);
}

void on_Kompid_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_kompid_changed != NULL)
		dataStruct->fcn_kompid_changed(protonum);
}

void on_ErrMessage_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_errmsg_changed != NULL)
		dataStruct->fcn_errmsg_changed(protonum);
}

void on_Regler_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_regler_changed != NULL)
		dataStruct->fcn_regler_changed(protonum);
}

void on_Diparam_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_diparam_changed != NULL)
		dataStruct->fcn_diparam_changed(protonum);
}

void on_Measdata_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_measdata_changed != NULL)
		dataStruct->fcn_measdata_changed(protonum);
}

void on_Status_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_status_changed != NULL)
		dataStruct->fcn_status_changed(protonum);
}

void on_MsConfig_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_msconfig_changed != NULL)
		dataStruct->fcn_msconfig_changed(protonum);
}

void on_Setz_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_setz_changed != NULL)
		dataStruct->fcn_setz_changed(protonum);
}

void on_Internals_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_internals_changed != NULL)
		dataStruct->fcn_internals_changed(protonum);
}

void on_WLAN_Password_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_wlan_password_changed != NULL)
		dataStruct->fcn_wlan_password_changed(protonum);
}

void on_PWR_Countdown_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_pwr_countdown_changed != NULL)
		dataStruct->fcn_pwr_countdown_changed(protonum);
}

void on_USB_Stick_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_usb_stick_changed != NULL)
		dataStruct->fcn_usb_stick_changed(protonum);
}

void on_Scriptstate_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_scriptstate_changed != NULL)
		dataStruct->fcn_scriptstate_changed(protonum);
}

void on_Scriptcommand_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_scriptcommand_changed != NULL)
		dataStruct->fcn_scriptcommand_changed(protonum);
}

void on_Scriptrequest_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_scriptrequest_changed != NULL)
		dataStruct->fcn_scriptrequest_changed(protonum);
}

void on_Local_Source_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_local_source_changed != NULL)
		dataStruct->fcn_local_source_changed(protonum);
}

// void on_WS_Connect_Changed(Proto_Num_e protonum)
// {
// 	if(dataStruct == NULL) return;
// 	if(dataStruct->fcn_ws_connect_changed != NULL) dataStruct->fcn_ws_connect_changed(protonum);
// }

void on_Kennlinie_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_kennlinie_changed != NULL)
		dataStruct->fcn_kennlinie_changed(protonum);
}

void on_Send_Overflow(void)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_send_overflow != NULL)
		dataStruct->fcn_send_overflow();
}

void on_Recv_Error(COM_Err_e err)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_recv_error != NULL)
		dataStruct->fcn_recv_error((int8_t)err);
}

void on_Reset_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_reset_changed != NULL)
		dataStruct->fcn_reset_changed();
}

void on_Update_Changed(Proto_Num_e protonum)
{
	if (dataStruct == NULL)
		return;
	if (dataStruct->fcn_update_changed != NULL)
		dataStruct->fcn_update_changed(protonum);
}
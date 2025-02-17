#ifndef DRIVER_D_COM_H_
#define DRIVER_D_COM_H_

/***********************************************************
 * Includes
 ***********************************************************/
#include <stdint.h>
/***********************************************************
 * Defines
 ***********************************************************/
#define RCV_BUF_SIZE 512
// #define PROTO_COUNT 57 // number of protocol types

#define RCV_BYTE_TIMEOUT 10

#define UART_ENDIANNESS BIG_ENDIAN // RAT: Falsch, nicht zu berücksichtigen. Das Bytezuordnungs-Model ist Little-Endian wie immer, aber JOH hat das für einzelnen Structure-Members gedacht
								   // V: RAT: False, not to be considered. The byte assignment model is little-endian as usual, but JOH meant this for individual structure members.
/* ERROR_MSG_LEVEL
 * RECV_ERROR_MSG_LEVEL_0 - no error reporting for the recieve queue errors (customer version)
 * RECV_ERROR_MSG_LEVEL_1 - reports recieve queue errors
 * RECV_ERROR_MSG_LEVEL_2 - reports recieve queue errors + packet mismatch(wrong check sum) errors
 */
#define RECV_ERROR_MSG_LEVEL_0

/***********************************************************
 * Data types, enums & structs
 ***********************************************************/

/**
 * Protocoll(UART) paramter numbers
 * count will be same as the member count in com_drv struct
 */
typedef enum
{
	NA_e = -1,
	IDENT = 0,
	PARAMETER,
	COMP_ID,
	ERRORMSG,
	REGLER_PM,
	REGLER_RIM,
	REGLER_PVM,
	PARAM_DI1,
	PARAM_DI2,
	PARAM_DI3,
	PARAM_DI4,
	PARAM_DI5,
	PARAM_DI6,
	PARAM_DI7,
	PARAM_DI8,
	PARAM_DI9,
	PARAM_DI10,
	PARAM_DI11,
	PARAM_DI12,
	PARAM_DI13,
	PARAM_DI14,
	PARAM_DI15,
	PARAM_DI16,
	LAN_INTERFACE,
	MESSDATEN,
	NT_STATUS,
	MSCONFIG,
	MESS_MS1,
	MESS_MS2,
	MESS_MS3,
	MESS_MS4,
	MESS_MS5,
	MESS_MS6,
	MESS_MS7,
	MESS_MS8,
	MESS_MS9,
	MESS_MS10,
	MESS_MS11,
	MESS_MS12,
	MESS_MS13,
	MESS_MS14,
	MESS_MS15,
	MESS_MS16,
	U_SETZ,
	I_SETZ,
	UMPP_SETZ,
	IMPP_SETZ,
	P_SETZ,
	RI_SETZ,
	OVP,
	OCP,
	T_OCP,
	UVP,
	T_UVP,
	U_SLOPE,
	I_SLOPE,
	AI_FILTER,
	U_LIMIT,
	I_LIMIT,
	FOLDBACK,
	FOLDBACKTM,
	RELASE,
	ODELAY,
	TEN,
	DATALOGGER,
	WLAN_PW,
	PWR_COUNTDOWN,
	USB_STICK,
	SCRIPTSTATE,
	SCRIPTCOMMAND,
	SCRIPTREQUEST,
	LOCAL_SOURCE,
	POL_MODE,
	// WS_CONNECT,
	UI_CURVE,
	SOFT_RESET,
	UPDATE_START,
	UPDATE_DATA,
	UPDATE_END,
	__PROTO_COUNT // it should always be in the last position!
} Proto_Num_e;

typedef struct
{
	uint16_t prototype;
	uint16_t payloadlen;
} Protocol_t;

typedef enum
{
	Packet_Type_NA = 0x7FFF,   // V: dont know
	Packet_Type_DATA = 0x0013, // V: this is data word
	Packet_Type_ACK = 0x0014,  // V: this is ack  word
	Packet_Type_ERR = 0x0017   // V: this is error word
} Packet_type_e;

typedef struct
{
	uint16_t payloadtype : 15; // payload type (protocol type)
	uint16_t req : 1;		   // Req-Bit
} Data_type_t;

typedef struct
{							  // 2 Word
	Packet_type_e packettype; // Data-Word: 0x0013, Ack-Word: 0x0014, Error-Word: 0x0017
	Data_type_t datatype;	  // payload type (message type)
} PaketHdr_t;

typedef struct
{						  // 2 Word
	PaketHdr_t packethdr; // Data-Word: 0x0013, Ack-Word: 0x0014, Error-Word: 0x0017
	void *payload;		  // V: this is the variable that stores the actual data that is taken from STM32. remaining all are confirmation variables
	uint32_t cs;
} Packet_t;

typedef enum
{
	COM_Resv_State_HDR = 0,
	COM_Resv_State_Type,
	COM_Resv_State_Data,
	COM_Resv_State_CS
} COM_Recv_state_e;

typedef struct
{
	char rx_buffer[RCV_BUF_SIZE]; // RX-buffer (linear buffer. resets after each complete packet received)
	uint16_t rd_ptr;			  // Read pointer (pionts to last read byte)
	uint16_t wr_ptr;			  // Write pointer (pionts to last written byte into buffer)
	uint16_t state;
} COM_Recv_queue_t;

typedef enum
{
	Err_RecvByte_Timeout = 1,
	Err_RecvBuff_Overflow = 2,
	Err_PacketType_Unknown = 3,
	Err_Prototype_Unknown = 4,
	Err_CS_Wrong = 5,
} COM_Err_e;

// Additional MIZ
typedef enum
{
	RESET = 0,
	SET = !RESET
} FlagStatus,
	ITStatus;

/***********************************************************
 * Function prototypes
 ************************************************************/

#endif /* DRIVER_D_COM_H_ */

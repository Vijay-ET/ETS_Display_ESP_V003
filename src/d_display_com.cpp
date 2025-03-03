
/***********************************************************
 * Includes
 ***********************************************************/
#include <ESP8266WiFi.h>

#include <stddef.h>
#include <string.h>

#include "d_display_com.h"
#include "a_display_com.h"
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

/**
 * A linear queue for receiving of one packet. It's reset when the complete packet is received or when timeout occurs
 * V: It is just a temporary stoarage area. after certain conditions the data will be copied to recvpacket struct
 */
static COM_Recv_queue_t COM_RCV;

/**
 * timestamp of each byte received
 */
static Timer_t byte_rcv_timestamp = 0;

/**
 * Module variable for save the received protocol nummer between the states switching
 */
static Proto_Num_e protonum = NA_e;

/**
 * Module variable for save the received packet data between the states switching
 *    V: This is permanent storage area(after this data is not copied to anywhere for now.)
 * // V: it is struct of type Packet_t that contains 3 variables packethdr, payload,cs
 * // V: packethdr is a struct of type PaketHdr_t that contains 2 more variables of types Packet_type_e && Data_type_t
 * // V: Packet_type_e is a struct that holds 4 variables those decides whta type of packet we have received
 * // V: Data_type_t is a struct that holds 2 bitfield variables 1-> payloadtype. 2-> req.
 */
static Packet_t recvpacket = {.packethdr = {.packettype = Packet_Type_NA, .datatype = {.payloadtype = 0, .req = 0}}, .payload = NULL, .cs = 0};
// static Packet_t recvpacket;

/***********************************************************
 * Private data types
 ***********************************************************/

/***********************************************************
 * Private function prototypes
 ***********************************************************/

/**
 * Sets the state macihne to the intitial state and resets the sm variables
 * @param void -
 * @return void
 */
static void reset_queue(void);

/**
 * Calculates the checksum of packet
 * @param packet - the package of which the cs must be calculated
 * @return 32bit checksum of the packet
 */
static uint32_t calc_checksum(Packet_t *packet, uint16_t payload_len);

/**
 * Looks up for the protocoll number in the protocol table
 * // V: which member of the com_drv has been received.
 * @param protdata - protocol byte value
 * @return protocol number
 */
static Proto_Num_e lookup_proto(uint16_t protdata);

/***********************************************************
 * Public function definitions
 ***********************************************************/

/**
 * It is called from IRQ handler at each byte that arrives.
 * V: It is called from COM_Handler to handle Input data Stream Coming from the STM32
 * @param wert - 1 byte in 16 bit placeholder (due to uc architecture)
 * @return void
 */
extern bool Wifi_ready;
void D_DataReceived(uint16_t wert)
{
  ESP.wdtFeed();	
  // wert -> 1 byte
  uint16_t packet_type;
  uint32_t cs_calc;

  // Check data stream for timeouts
  // When sm is triggered (header is received), the intervals between single bytes should not exceed RCV_BYTE_TIMEOUT value
  if (COM_RCV.state != COM_Resv_State_HDR && (timer_get_time() - byte_rcv_timestamp > RCV_BYTE_TIMEOUT))
  {
    reset_queue();
#if defined(RECV_ERROR_MSG_LEVEL_1) || defined(RECV_ERROR_MSG_LEVEL_2)
    D_Error(Err_RecvByte_Timeout);
#endif
  }

  byte_rcv_timestamp = timer_get_time();

  // 1 Byte is received. Append it to the rx buffer
  COM_RCV.rx_buffer[COM_RCV.wr_ptr] = wert; // V: COM_RCV is a Variable of Type COM_Recv_queue_t Struct which Contains 4 members (Buffer, rd_ptr, wt_ptr, State)

  // incerement write pointer
  COM_RCV.wr_ptr++;

  switch (COM_RCV.state)
  {
  case COM_Resv_State_HDR: // V: search for the 2 header bytes - 0x0013, 0x0014 or 0x0017  // V: it will be this state by default(1st time (or) after reset_queue())
                           // Serial.println("COM_Resv_State_HDR");
    // if two bytes are available in queue
    if ((COM_RCV.rd_ptr + 1) < COM_RCV.wr_ptr)
    {

      packet_type = *(uint16_t *)&COM_RCV.rx_buffer[COM_RCV.rd_ptr]; // V: the header in COM_RCV queue is copied here into packet_type variabe
      if (packet_type == Packet_Type_DATA || packet_type == Packet_Type_ACK)
      {
        COM_RCV.rd_ptr = COM_RCV.wr_ptr;
        // V: recvpacket is a variable of type Packet_t and Packet_t is a Struct which contains 3 mems(packethdr, payload,cs)
        // V: packet_type variable is copied into recvpacket struct here and in every case
        recvpacket.packethdr.packettype = (Packet_type_e)packet_type;
        COM_RCV.state = COM_Resv_State_Type;
      }
      else
      {
        COM_RCV.rd_ptr++;
      }
    }
    break;

  case COM_Resv_State_Type: // aquisite the type
    // if two bytes are available in queue
    if ((COM_RCV.rd_ptr + 1) < COM_RCV.wr_ptr)
    {

      recvpacket.packethdr.datatype = *(Data_type_t *)&COM_RCV.rx_buffer[COM_RCV.rd_ptr]; // V: the state type in queue will be copied here to recvpacket
      COM_RCV.rd_ptr = COM_RCV.wr_ptr;

      // lookup for protocol number in protocol types table
      protonum = lookup_proto(recvpacket.packethdr.datatype.payloadtype);

      // V: check if this is a data packet or req/ack packet (or err packet)
      if (recvpacket.packethdr.packettype == Packet_Type_DATA)
      {
        if (protonum != NA_e)
        { // is a valid protocol?
          COM_RCV.state = COM_Resv_State_Data;
        }
        else
        {
          // D_PacketDataCorruptedReceived(prototype);
          //  discard data and ignore data in linear buffer, resetting the byte queue and switching sm to initial state
          reset_queue();
          Clean_serial_buffer();
          
#if defined(RECV_ERROR_MSG_LEVEL_1) || defined(RECV_ERROR_MSG_LEVEL_2)
          D_Error(Err_Prototype_Unknown);
#endif
        }
      }
      else if (recvpacket.packethdr.packettype == Packet_Type_ACK) // no data and no checksum in the ack/nack packets
      {
        if (recvpacket.packethdr.datatype.req == SET)
        {
          D_PacketNACKReceived(protonum);
        }
        else
        {
          D_PacketACKReceived(protonum);
        }

        // the job is done via upper instance a_netzteil_com. Reset queue to initial state
        reset_queue();
      }
      else if (recvpacket.packethdr.packettype == Packet_Type_ERR)
      {
#if defined(RECV_ERROR_MSG_LEVEL_1) || defined(RECV_ERROR_MSG_LEVEL_2)
        D_Error(Err_PacketType_Unknown);
#endif
        reset_queue();
      }
      else
      {
        // Unknown data type or packet mismatch. Reset state machine to initial state
#if defined(RECV_ERROR_MSG_LEVEL_1) || defined(RECV_ERROR_MSG_LEVEL_2)
        D_Error(Err_PacketType_Unknown);
#endif
        reset_queue();
      }
    }
    break;

  case COM_Resv_State_Data:                                                 // aquisite the data payload from the stream
    if (COM_RCV.rd_ptr + ProtoTypes[protonum].payloadlen == COM_RCV.wr_ptr) // entire payload is received
    {
      recvpacket.payload = &COM_RCV.rx_buffer[COM_RCV.rd_ptr]; // V: recvpacket.payload pointer will point to the data in the COM_RCV
      COM_RCV.rd_ptr = COM_RCV.wr_ptr;
      COM_RCV.state = COM_Resv_State_CS;
    }
    break;

  case COM_Resv_State_CS: // aquisition of the checksum
    // Checksum
    if (COM_RCV.rd_ptr + sizeof(uint32_t) == COM_RCV.wr_ptr) // entire cs bytes are received
    {
      recvpacket.cs = *(uint32_t *)&COM_RCV.rx_buffer[COM_RCV.rd_ptr]; // V: The Checksum Address will  be copied to COM_RCV cs pointer
      // calculate the checksum of the received packet
      cs_calc = calc_checksum(&recvpacket, ProtoTypes[protonum].payloadlen);

      if (cs_calc == recvpacket.cs)
      {
        // process packet payload
        // V: in this payload_received will be called that will assign recvpacket data to com_drv struct
        // V: after that com_drv values will be copied to NtParam struct
        D_PacketDataValidReceived(protonum, recvpacket.payload);
      }
      else
      {
#ifdef RECV_ERROR_MSG_LEVEL_2
        D_Error(Err_CS_Wrong);
#endif
        D_PacketDataCorruptedReceived(protonum);
      }

      // entire packet is received. Reset state macihne to the initial state

      reset_queue();
    }
    break;

  default:
    break;
  }

  // check if buffer is full
  if (COM_RCV.wr_ptr >= RCV_BUF_SIZE) // Error buffer is full. No known frame data within RCV_BUF_SIZE available
  {
#if defined(RECV_ERROR_MSG_LEVEL_1) || defined(RECV_ERROR_MSG_LEVEL_2)
    D_Error(Err_RecvBuff_Overflow);
#endif
    reset_queue();
    return;
  }
}

/*  V: D_SendPacket sends the data over UART asynchronously.
 *  V: The FIFO buffer has a size of 128 bytes. If the buffer is full, Serial.write blocks until some space becomes available.
 *  V: Serial.availableForWrite() returns the number of free bytes available
 */
void D_SendPacket(Packet_t *sndpacket, uint16_t payload_len) // V: Used to send ACK, NACK, REQ packets to the STM32
{
  // send packet header
  Serial.write((uint8_t *)&(sndpacket->packethdr.packettype), 2);
  Serial.write((uint8_t *)&(sndpacket->packethdr.datatype), 2);

  // if this is a data packet, send the payload and the checksum
  if (payload_len != 0)
  {
    // send the payload data

    uint32_t *payload32 = (uint32_t *)sndpacket->payload;

    for (uint16_t i = 0; i < (payload_len / 4); i++)
    {
      // uart_SendArray(COM6, (uint8_t*)&payload32[i] , sizeof(uint32_t), UART_ENDIANNESS);
      Serial.write((uint8_t *)&payload32[i], sizeof(uint32_t));
    }

    // calc the checksum and assign to packet variable (for debug only)
    sndpacket->cs = calc_checksum(sndpacket, payload_len);

    // send the checksum
    // uart_SendArray(COM6, (uint8_t*)&sndpacket->cs , sizeof(uint32_t), UART_ENDIANNESS);
    Serial.write((uint8_t *)&sndpacket->cs, sizeof(uint32_t));
  }
}

/***********************************************************
 * Private function definitions
 ***********************************************************/

void reset_queue(void)
{
  // V: clearing the Serial rx buffer for every reset queue
  // Clean_serial_buffer();
  protonum = (Proto_Num_e)0;
  memset(&COM_RCV, 0, sizeof(COM_Recv_queue_t));
  memset(&recvpacket, 0, sizeof(Packet_t));
  // switch sm to header search
  COM_RCV.state = COM_Resv_State_HDR;
}

uint32_t calc_checksum(Packet_t *packet, uint16_t payload_len)
{
  uint32_t *payload32 = (uint32_t *)packet->payload;
  // uint32_t cs_calc = *(uint32_t *)&packet->packethdr;
  uint32_t cs_calc;
  cs_calc = *(uint16_t *)&packet->packethdr.datatype << 16;
  cs_calc += *(uint16_t *)&packet->packethdr.packettype;

  for (uint32_t var = 0; var < (payload_len / 4); var++)
  {
    cs_calc += payload32[var];
  }
  cs_calc = ~cs_calc;
  return cs_calc;
}

void Clean_serial_buffer(void)
{
  ESP.wdtFeed();
#ifdef DEBUG_MESSAGES_ACTIVE
  Serial_Printing_Port.println("Cleaing the serial buffers.");
#endif
  while(Serial.available())
  {
    ESP.wdtFeed();
    Serial.read();
  }
}

Proto_Num_e lookup_proto(uint16_t protdata)
{
  Proto_Num_e ret = NA_e;

  if (protdata == ProtoTypes[IDENT].prototype)
    ret = IDENT;
  else if (protdata == ProtoTypes[PARAMETER].prototype)
    ret = PARAMETER;
  else if (protdata == ProtoTypes[COMP_ID].prototype)
    ret = COMP_ID;
  else if (protdata == ProtoTypes[ERRORMSG].prototype)
    ret = ERRORMSG;
  else if (protdata == ProtoTypes[REGLER_PM].prototype)
    ret = REGLER_PM;
  else if (protdata == ProtoTypes[REGLER_RIM].prototype)
    ret = REGLER_RIM;
  else if (protdata == ProtoTypes[REGLER_PVM].prototype)
    ret = REGLER_PVM;
  else if (protdata == ProtoTypes[PARAM_DI1].prototype)
    ret = PARAM_DI1;
  else if (protdata == ProtoTypes[PARAM_DI2].prototype)
    ret = PARAM_DI2;
  else if (protdata == ProtoTypes[PARAM_DI3].prototype)
    ret = PARAM_DI3;
  else if (protdata == ProtoTypes[PARAM_DI4].prototype)
    ret = PARAM_DI4;
  else if (protdata == ProtoTypes[PARAM_DI5].prototype)
    ret = PARAM_DI5;
  else if (protdata == ProtoTypes[PARAM_DI6].prototype)
    ret = PARAM_DI6;
  else if (protdata == ProtoTypes[PARAM_DI7].prototype)
    ret = PARAM_DI7;
  else if (protdata == ProtoTypes[PARAM_DI8].prototype)
    ret = PARAM_DI8;
  else if (protdata == ProtoTypes[PARAM_DI9].prototype)
    ret = PARAM_DI9;
  else if (protdata == ProtoTypes[PARAM_DI10].prototype)
    ret = PARAM_DI10;
  else if (protdata == ProtoTypes[PARAM_DI11].prototype)
    ret = PARAM_DI11;
  else if (protdata == ProtoTypes[PARAM_DI12].prototype)
    ret = PARAM_DI12;
  else if (protdata == ProtoTypes[PARAM_DI13].prototype)
    ret = PARAM_DI13;
  else if (protdata == ProtoTypes[PARAM_DI14].prototype)
    ret = PARAM_DI14;
  else if (protdata == ProtoTypes[PARAM_DI15].prototype)
    ret = PARAM_DI15;
  else if (protdata == ProtoTypes[PARAM_DI16].prototype)
    ret = PARAM_DI16;
  else if (protdata == ProtoTypes[LAN_INTERFACE].prototype)
    ret = LAN_INTERFACE;
  else if (protdata == ProtoTypes[MESSDATEN].prototype)
    ret = MESSDATEN;
  else if (protdata == ProtoTypes[NT_STATUS].prototype)
    ret = NT_STATUS;
  else if (protdata == ProtoTypes[MSCONFIG].prototype)
    ret = MSCONFIG;
  else if (protdata == ProtoTypes[MESS_MS1].prototype)
    ret = MESS_MS1;
  else if (protdata == ProtoTypes[MESS_MS2].prototype)
    ret = MESS_MS2;
  else if (protdata == ProtoTypes[MESS_MS3].prototype)
    ret = MESS_MS3;
  else if (protdata == ProtoTypes[MESS_MS4].prototype)
    ret = MESS_MS4;
  else if (protdata == ProtoTypes[MESS_MS5].prototype)
    ret = MESS_MS5;
  else if (protdata == ProtoTypes[MESS_MS6].prototype)
    ret = MESS_MS6;
  else if (protdata == ProtoTypes[MESS_MS7].prototype)
    ret = MESS_MS7;
  else if (protdata == ProtoTypes[MESS_MS8].prototype)
    ret = MESS_MS8;
  else if (protdata == ProtoTypes[MESS_MS9].prototype)
    ret = MESS_MS9;
  else if (protdata == ProtoTypes[MESS_MS10].prototype)
    ret = MESS_MS10;
  else if (protdata == ProtoTypes[MESS_MS11].prototype)
    ret = MESS_MS11;
  else if (protdata == ProtoTypes[MESS_MS12].prototype)
    ret = MESS_MS12;
  else if (protdata == ProtoTypes[MESS_MS13].prototype)
    ret = MESS_MS13;
  else if (protdata == ProtoTypes[MESS_MS14].prototype)
    ret = MESS_MS14;
  else if (protdata == ProtoTypes[MESS_MS15].prototype)
    ret = MESS_MS15;
  else if (protdata == ProtoTypes[MESS_MS16].prototype)
    ret = MESS_MS16;
  else if (protdata == ProtoTypes[U_SETZ].prototype)
    ret = U_SETZ;
  else if (protdata == ProtoTypes[I_SETZ].prototype)
    ret = I_SETZ;
  else if (protdata == ProtoTypes[UMPP_SETZ].prototype)
    ret = UMPP_SETZ;
  else if (protdata == ProtoTypes[IMPP_SETZ].prototype)
    ret = IMPP_SETZ;
  else if (protdata == ProtoTypes[P_SETZ].prototype)
    ret = P_SETZ;
  else if (protdata == ProtoTypes[RI_SETZ].prototype)
    ret = RI_SETZ;
  else if (protdata == ProtoTypes[OVP].prototype)
    ret = OVP;
  else if (protdata == ProtoTypes[OCP].prototype)
    ret = OCP;
  else if (protdata == ProtoTypes[T_OCP].prototype)
    ret = T_OCP;
  else if (protdata == ProtoTypes[UVP].prototype)
    ret = UVP;
  else if (protdata == ProtoTypes[T_UVP].prototype)
    ret = T_UVP;
  else if (protdata == ProtoTypes[FOLDBACK].prototype)
    ret = FOLDBACK;
  else if (protdata == ProtoTypes[FOLDBACKTM].prototype)
    ret = FOLDBACKTM;
  else if (protdata == ProtoTypes[RELASE].prototype)
    ret = RELASE;
  else if (protdata == ProtoTypes[U_SLOPE].prototype)
    ret = U_SLOPE;
  else if (protdata == ProtoTypes[I_SLOPE].prototype)
    ret = I_SLOPE;
  else if (protdata == ProtoTypes[AI_FILTER].prototype)
    ret = AI_FILTER;
  else if (protdata == ProtoTypes[U_LIMIT].prototype)
    ret = U_LIMIT;
  else if (protdata == ProtoTypes[I_LIMIT].prototype)
    ret = I_LIMIT;
  else if (protdata == ProtoTypes[POL_MODE].prototype)
    ret = POL_MODE;
  else if (protdata == ProtoTypes[UI_CURVE].prototype)
    ret = UI_CURVE;
  else if (protdata == ProtoTypes[ODELAY].prototype)
    ret = ODELAY;
  else if (protdata == ProtoTypes[TEN].prototype)
    ret = TEN;
  else if (protdata == ProtoTypes[DATALOGGER].prototype)
    ret = DATALOGGER;
  else if (protdata == ProtoTypes[WLAN_PW].prototype)
    ret = WLAN_PW;
  else if (protdata == ProtoTypes[PWR_COUNTDOWN].prototype)
    ret = PWR_COUNTDOWN;
  else if (protdata == ProtoTypes[USB_STICK].prototype)
    ret = USB_STICK;
  else if (protdata == ProtoTypes[SCRIPTSTATE].prototype)
    ret = SCRIPTSTATE;
  else if (protdata == ProtoTypes[SCRIPTCOMMAND].prototype)
    ret = SCRIPTCOMMAND;
  else if (protdata == ProtoTypes[SCRIPTREQUEST].prototype)
    ret = SCRIPTREQUEST;
  else if (protdata == ProtoTypes[LOCAL_SOURCE].prototype)
    ret = LOCAL_SOURCE;
  else if (protdata == ProtoTypes[SOFT_RESET].prototype)
    ret = SOFT_RESET;
  else if (protdata == ProtoTypes[UPDATE_START].prototype)
    ret = UPDATE_START;
  else if (protdata == ProtoTypes[UPDATE_DATA].prototype)
    ret = UPDATE_DATA;
  else if (protdata == ProtoTypes[UPDATE_END].prototype)
    ret = UPDATE_END;
  else
    ret = NA_e;

  return ret;
}

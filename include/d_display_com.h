#ifndef DRIVER_D_NETZTEIL_COM_H_
#define DRIVER_D_NETZTEIL_COM_H_

/***********************************************************
 * Includes
 ***********************************************************/

#include "d_com.h"

/***********************************************************
 * Defines
 ***********************************************************/

/***********************************************************
 * Data types, enums & structs
 ***********************************************************/

/***********************************************************
 * Function prototypes
 ************************************************************/
/*
 * Interrupt driven function for aquisition of data from the active uart interface
 * Here data will be stored in recvpacket and then trasefered to com_drv in the CS case child functions
 *
 */
void D_DataReceived(uint16_t wert);

/**
 * IPC function for transfer of data from a_netzteil.com to d_netzteil.com
 * @param sndpacket - packet to send
 * @return void
 */
void D_SendPacket(Packet_t *sndpacket, uint16_t payload_len); // IPC function for transferr data

#endif /* DRIVER_D_NETZTEIL_COM_H_ */
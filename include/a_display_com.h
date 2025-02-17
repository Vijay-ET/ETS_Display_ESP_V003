
#ifndef APPLICATION_A_NETZTEIL_COM_H_
#define APPLICATION_A_NETZTEIL_COM_H_

/***********************************************************
 * Includes
 ***********************************************************/

#include "a_com.h"
#include "d_display_com.h"

/***********************************************************
 * Defines
 ***********************************************************/

/***********************************************************
 * Data types, enums & structs
 ***********************************************************/

/***********************************************************
 * Function prototypes
 ************************************************************/

/**
 * Initialize a display driver with default values.
 * It is used to surly have known values in the fields and not memory junk.
 * After it you can set the fields.
 * @param driver pointer to driver variable to initialize
 */
void COM_Driver_Init(com_drv_t *driver);

/**
 * Register an initialized display driver.
 * @param driver pointer to an initialized 'disp_drv_t' variable (can be local variable)
 * @return pointer to the new display or NULL on error
 */
comdrvErr_e COM_Driver_Register(com_drv_t *driver);

/**
 * COM_ interface functions for interact with the a_display_com driver.
 */
void COM_Handler(void);

/**
 * Enqueue a packet to be send
 * @param protocoltype - protcol paramter type
 * @return void
 */
void COM_SendParameter(Proto_Num_e num);

/**
 * Enqueue a request data packet to be send
 * @param protocoltype - protcol paramter type
 * @return void
 */
void COM_GetParameter(Proto_Num_e num);

/**
 * IPC function for transfer of data from a_netzteil.com to d_netzteil.com
 * @param received packet from instance bellow -
 * @return void
 */
void D_PacketDataValidReceived(Proto_Num_e num, void *payload);

/**
 * IPC function for transfer of data from a_netzteil.com to d_netzteil.com
 * @param received packet from instance bellow -
 * @return void
 */
void D_PacketDataCorruptedReceived(Proto_Num_e num);

/**
 * IPC function for transfer of data from a_netzteil.com to d_netzteil.com
 * @param protocoltype -
 * @return void
 */
void D_PacketACKReceived(Proto_Num_e num);

/**
 * IPC function for transfer of data from a_netzteil.com to d_netzteil.com
 * @param protocoltype -
 * @return void
 */
void D_PacketNACKReceived(Proto_Num_e num);



/*
* Function that will claer the UART rx buffer by discarding the bytes.
* This will be useful in all the cases. Reduces the dependency of STM transmitting.
* @param void
* @return void
*/
void Clean_serial_buffer(void);
#endif /* APPLICATION_A_NETZTEIL_COM_H_ */

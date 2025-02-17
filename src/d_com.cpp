/***********************************************************
 * Includes
 ***********************************************************/

#include "d_com.h"
#include "a_com.h"
#include "Serial_prints.h"

/***********************************************************
 * Global variables
 ***********************************************************/

/* This a array of struct declaration that is used to
 *  handle which data(com_drv member) is received from STM and stores in a variable
 *  and that is used to to update particular member through call backs and also
 * used to send same data to the front-end */
Protocol_t ProtoTypes[__PROTO_COUNT] = {
    // {prot code, payload length},
    {0x0000, sizeof(T_Ident)},           // IDENT
    {0x0001, sizeof(T_DevParameter)},    // PARAMETER
    {0x0002, sizeof(T_Compid)},          // COMP_ID
    {0x0003, sizeof(T_ErrMessage)},      // ERRORMSG
    {0x0010, sizeof(T_ReglerParam)},     // REGLER_PM
    {0x0011, sizeof(T_ReglerParam)},     // REGLER_RIM
    {0x0012, sizeof(T_ReglerParam)},     // REGLER_PVM
    {0x0020, sizeof(T_DiParams)},        // PARAM_DI1
    {0x0021, sizeof(T_DiParams)},        // PARAM_DI2
    {0x0022, sizeof(T_DiParams)},        // PARAM_DI3
    {0x0023, sizeof(T_DiParams)},        // PARAM_DI4
    {0x0024, sizeof(T_DiParams)},        // PARAM_DI5
    {0x0025, sizeof(T_DiParams)},        // PARAM_DI6
    {0x0026, sizeof(T_DiParams)},        // PARAM_DI7
    {0x0027, sizeof(T_DiParams)},        // PARAM_DI8
    {0x0028, sizeof(T_DiParams)},        // PARAM_DI9
    {0x0029, sizeof(T_DiParams)},        // PARAM_DI10
    {0x002a, sizeof(T_DiParams)},        // PARAM_DI11
    {0x002b, sizeof(T_DiParams)},        // PARAM_DI12
    {0x002c, sizeof(T_DiParams)},        // PARAM_DI13
    {0x002d, sizeof(T_DiParams)},        // PARAM_DI14
    {0x002e, sizeof(T_DiParams)},        // PARAM_DI15
    {0x002f, sizeof(T_DiParams)},        // PARAM_DI16
    {0x0030, sizeof(T_LanInterface)},    // LAN_INTERFACE params - MAC,IP,GATEWAY,DYNAMIC IP
    {0x0100, sizeof(T_Messdaten)},       // MESSDATEN
    {0x0101, sizeof(T_Status)},          // STATUS
    {0x0102, sizeof(T_MSConfig)},        // MSCONFIG
    {0x0120, sizeof(T_Messdaten)},       // MESS_MS1
    {0x0121, sizeof(T_Messdaten)},       // MESS_MS2
    {0x0122, sizeof(T_Messdaten)},       // MESS_MS3
    {0x0123, sizeof(T_Messdaten)},       // MESS_MS4
    {0x0124, sizeof(T_Messdaten)},       // MESS_MS5
    {0x0125, sizeof(T_Messdaten)},       // MESS_MS6
    {0x0126, sizeof(T_Messdaten)},       // MESS_MS7
    {0x0127, sizeof(T_Messdaten)},       // MESS_MS8
    {0x0128, sizeof(T_Messdaten)},       // MESS_MS9
    {0x0129, sizeof(T_Messdaten)},       // MESS_MS10
    {0x012a, sizeof(T_Messdaten)},       // MESS_MS11
    {0x012b, sizeof(T_Messdaten)},       // MESS_MS12
    {0x012c, sizeof(T_Messdaten)},       // MESS_MS13
    {0x012d, sizeof(T_Messdaten)},       // MESS_MS14
    {0x012e, sizeof(T_Messdaten)},       // MESS_MS15
    {0x012f, sizeof(T_Messdaten)},       // MESS_MS16
    {0x0200, sizeof(float)},             // U_SETZ
    {0x0201, sizeof(float)},             // I_SETZ
    {0x0202, sizeof(float)},             // UMPP_SETZ
    {0x0203, sizeof(float)},             // IMPP_SETZ
    {0x0204, sizeof(float)},             // P_SETZ
    {0x0205, sizeof(float)},             // RI_SETZ
    {0x0300, sizeof(float)},             // OVP
    {0x0301, sizeof(float)},             // OCP
    {0x0302, sizeof(float)},             // T_OCP
    {0x0303, sizeof(float)},             // UVP
    {0x0304, sizeof(float)},             // T_UVP
    {0x0305, sizeof(float)},             // U_SLOPE
    {0x0306, sizeof(float)},             // I_SLOPE
    {0x0307, sizeof(T_AnalogInterface)}, // AI_FILTER
    {0x0308, sizeof(float)},             // U_LIMIT
    {0x0309, sizeof(float)},             // I_LIMIT
    {0x0310, sizeof(uint32_t)},          // FOLDBACK
    {0x0311, sizeof(float)},             // FOLDBACKTM
    // New Protocol Messages
    {0x0312, sizeof(uint32_t)},         // Internal Parameters: Remember last Settings
    {0x0313, sizeof(uint32_t)},         // Internal Parameters: Output on Delay
    {0x0314, sizeof(uint32_t)},         // Internal Parameters: T Enable
    {0x0315, sizeof(uint32_t)},         // Internal Parameters: Datalogger
    {0x0316, sizeof(uint32_t)},         // WLAN password
    {0x0317, sizeof(uint32_t)},         // Power State
    {0x0318, sizeof(uint32_t)},         // check if USB Stick is plugged in
    {0x0319, sizeof(uint32_t)},         // scriptstate (unloaded, loaded, running)
    {0x0320, sizeof(T_Script_command)}, // last 4 lines with script commands
    {0x0321, sizeof(uint32_t)},         // user or script mode request. 0 -> user 1 -> script
    {0x0322, sizeof(uint32_t)},         // Local source. 0 -> Front 1 -> WLAN
    // // {0x0320, sizeof(uint32_t)}, //1 if websocket received a connection
    // {0x0400, sizeof(T_Kennlinie)},// UI_CURVE
    {0x0323, sizeof(uint32_t)},      // POL_MODE - Polarity mode - Free/Standby - 1/0
    {0x0400, sizeof(T_Kennlinie)},   // UI_CURVE
    {0x04fe, 0},                     // SOFT_RESET
    {0x1000, sizeof(T_UpdateStart)}, // UPDATE_START
    {0x1001, sizeof(T_UpdateData)},  // UPDATE_DATA
    {0x1002, sizeof(uint32_t)},      // UPDATE_END (CITT16 checksum)
};

/***********************************************************
 * Module variables
 ***********************************************************/

/***********************************************************
 * Private data types
 ***********************************************************/

/***********************************************************
 * Private function prototypes
 ***********************************************************/

/***********************************************************
 * Public function definitions
 ***********************************************************/

/***********************************************************
 * Private function definitions
 ***********************************************************/

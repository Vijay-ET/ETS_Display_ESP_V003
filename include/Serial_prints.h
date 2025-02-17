#ifndef SERIAL_PRINTS_ALLOW
#define SERIAL_PRINTS_ALLOW

// #define SERIAL_PRINTS 

#ifdef SERIAL_PRINTS  
	#define Serial_Printing_Port Serial		  // V: Print in UART1 for Dev-Kit
#else
	#define Serial_Printing_Port Serial1	  // V: Print in UART2 for On-board module
#endif

// V: function that will print system information
void Print_system_info(void);
#endif
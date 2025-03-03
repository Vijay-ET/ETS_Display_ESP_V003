/***********************************************************
 *
 * * a_ws_com.cpp
 * SYSTART GmbH
 * MIZ, 30.06.2021
 *
 ***********************************************************/

/***********************************************************
 * Includes
 ***********************************************************/
#include "a_ws_com.h"
#include <string.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "a_display_com.h"
#include <map>
#include "Serial_prints.h"

/***********************************************************
 * Defines
 ***********************************************************/
#define SEND_BUFFER_SIZE    ((PLOT_DATA_SIZE * 4) + 1)  // PLOT_DATA_SIZE * 2 * uint16 + Messages_e

extern char *Wifi_string;
extern const char *Wifi_password;
extern bool Wifi_ready;

/***********************************************************
 * Global Variables
 ***********************************************************/
extern char* default_wifi_str;				

#if 0
//Accespoint   // V: This is main for Wi-Fi module
char* apSsid = Wifi_string;
const char* apPassword = Wifi_password;
#elif 2
//Accespoint   // V: This is main for Wi-Fi module
const char* apSsid = "ETS-Display";
const char* apPassword = "12345678";
#else
//Accespoint   // V: This is main for Wi-Fi module
const char* apSsid = "ETS-Onboard-LAN";
const char* apPassword = "12345678";
#endif

// V: creating a channel for reducing interface.
int channel = 6; // V: 1, 6, 11 are the channels that are less overlapping.
// V: Parameters while acting as a Station Mode
// WLAN    // V: Down Wi-fi's are Existing Local Ones

 const char* ssid = "ET SYSTEM INDIA_5G";      // V: Created to check if Wi-Fi module is connecting to our Wi-Fi
 const char* password = "ETSIN@2020";          // V: This Line is Password of the ET system INDIA Wi-Fi

extern AsyncWebServer server; // V: declared for enabling OTA updates.
// extern AsyncWebServer server(80);  // V: 80 is the default port for HTTP requests 
// V: Creating an Instance of Async Websocket and Intialising it with "/ws" End Point
// V: When a Client(Web Page) connects To this Server(our Module). It will use this End Point To Establish WS Connection
AsyncWebSocket webSocket("/ws");  
IPAddress apIP(192, 168, 0, 1);


MSGprio_t msgPrio;

DevParam_t devParam;                  // V: these are made extern in a_ws_com.h and inlcude in main.cpp that's why we can change them directly in main
      // V: This is Monitor Segment in both Touch display and website(front-end). Changing this will have direct effect 
Monitor_t monitor;                    // V: these are made extern in a_ws_com.h and inlcude in main.cpp that's why we can change them directly in main    
      // V: This is Monitor Segment in both Touch display and website(front-end). Changing this will have direct effect
Preset_t preset;                      // V: these are made extern in a_ws_com.h and inlcude in main.cpp that's why we can change them directly in main     
IntParam_t intParam;                  // V: these are made extern in a_ws_com.h and inlcude in main.cpp that's why we can change them directly in main
extern Netzteil_Parameters_t NtParam;
extern com_drv_t com_drv;
Status_t status;                      // V: these are made extern in a_ws_com.h and inlcude in main.cpp that's why we can change them directly in main
uint8_t count_pwr;  //value of counter (OffPend or OnPend)
uint8_t usb_active = false;
uint8_t state_script;  //scriptmode_state_e
uint8_t script_req;
// uint8_t ws_connect;
Script_command_t cmd_script;
Control_t control;                    // V: these are made extern in a_ws_com.h and inlcude in main.cpp that's why we can change them directly in main
XYPlotData_t xyPlotData;
MS_Status_t msstate;
Ident_t ident;
Compid_t compid;
Foldback_t FoldBack;    // V: New structure implemented to hold the Foldback & FoldbackTm.
extern uint8_t ms_available;
uint8_t source_local = SOURCE_FRONT;
extern Timer_t lastNoClientMessage ;
extern Timer_t Ws_last_Ping_Time;
extern Timer_t Ws_Ping_Time;
extern uint8_t counter;
bool counter_active = false;
bool counting = false;
Timer_t pendtimer;

// V: variables used to control the Wi-Fi and Websocket 
uint8_t keep_alive = 0;
bool server_running = false;
bool client_connected = false;
bool got_response = false;
// extern uint8_t prio_cnt;

// V: variable used to Track the connected clients.
int clients_num = 0;
int Ws_clean_count =0; // V: variable used to clean websocket resources.

extern Timer_t Time_STM_Last_msg_received;
extern bool STM_Freezed;

/***********************************************************
 * Lokale Funktionen
 ***********************************************************/
std::map<uint32_t, AsyncWebSocketClient*> clients; // V: Used to keep Track of clients
void on_WebSocket_Event(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);
void handle_Received_Message(uint8_t *data, size_t len);
void start_counter();
// void send_binaryData(uint8_t* data, uint16_t size);
uint32_t if_baud_i[] = {0, 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 57600, 62500, 115200};

/***********************************************************
 * Funktionsdefinitionen
 ***********************************************************/

/**
 * Init wifi as access-point or client
 * Init webserver
 * Init websocket communication
 * Set HTTP_GET and websocket callbacks
 * @param void
 * @return void
 */
void Init_Wifi()
{
  memset(msgPrio.prio, 0xFF, sizeof(msgPrio.prio));

#ifdef WIFI_MODE_AP    // V: This MACRO is used To make Wi-Fi mod as Access point mode ( Acts as LAN )
                       // V: In this Mode Server -> Wi-Fi Module   Client -> Web Page
 char pw[16] = {0};

  if(NtParam.wlanconfig.password){
	  //convert uint32_t password into char
	  sprintf(pw, "%d", NtParam.wlanconfig.password);
    // Serial_Printing_Port.println("Password received from STM: " + (String)pw);
  }
  else{
    // strncpy(a, "iqbal", sizeof(a) - 1);
    strncpy(pw, apPassword, sizeof(pw) - 1);
    // sprintf(pw, "%c", apPassword); //set default password at the beginning if no password is received yet from STM
    // Serial_Printing_Port.println("default password set: " + (String)pw);
  }
  
  WiFi.mode(WIFI_AP);  // V: If we want moudle To act as Station mode(Connect to local Wi-Fi) It will act if we chane the Macro to -> WIFI_STA
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));  // V: Parameters -> Local Ip(Ip assigned to module), Gateway(same as Local Ip), Subnet mask(Address Space)
    Serial_Printing_Port.println("Setting up AP");  
  #ifndef IS_ESP_DEV_BOARD
    // WiFi.softAP(apSsid, apPassword,1,0,1); //set apSSID, password, channel (default = 1), show (0) or hide (1) SSID, max connections
    WiFi.softAP(apSsid, (const char*)pw,1,0,1); //set apSSID, password, channel (default = 1), show or hide SSID, max connections
  #else
    WiFi.softAP(default_wifi_str, Wifi_password,6,0,3); //set apSSID, default password, channel 6, show (0) or hide (1) SSID, max connections(3)
     /* V: The above line parameters are 
                    1-> Service ID 
                    2-> Password
                    3-> Channel Number Used By the Access Point
                    4-> Wi-Fi mode of the Access Point
                    5-> No of Devices that can be Connected. */ 
  #endif
  
  #ifdef DEBUG_MESSAGES_ACTIVE
  //If connection successful show IP address in Serial_Printing_Port monitor
  Serial_Printing_Port.println("");
  Serial_Printing_Port.print("AP Name: ");
  Serial_Printing_Port.println(Wifi_string);
  Serial_Printing_Port.print("IP address: ");
  Serial_Printing_Port.println(WiFi.softAPIP());   //IP address assigned to your ESP
  Serial_Printing_Port.print("AP password: ");
  Serial_Printing_Port.println(WiFi.softAPPSK());  //password assigned to your ESP
  #endif

#else      // V: IF not Access Point Mode then Wi-Fi Module Acts as Station Mode ( Will Connect to an Existing Local Wi-Fi )
           // V: In This Mode Server -> Our Pc(Web Page)    Client -> Wi-Fi Module

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  #ifdef DEBUG_MESSAGES_ACTIVE
  Serial_Printing_Port.println("");
  #endif

  // Wait for connection
  Serial_Printing_Port.print("Connecting ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  #ifdef DEBUG_MESSAGES_ACTIVE
    Serial_Printing_Port.print(".");
  #endif
  }

  #ifdef DEBUG_MESSAGES_ACTIVE
  //If connection successful show IP address in Serial_Printing_Port monitor
  Serial_Printing_Port.println("");
  Serial_Printing_Port.print("Connected to ");
  Serial_Printing_Port.println(ssid);
  Serial_Printing_Port.print("IP address: ");
  Serial_Printing_Port.println(WiFi.localIP());  //IP address assigned to your ESP
  #endif
#endif
Serial_Printing_Port.println("Wi-Fi initialisations completed.");
  init_WS();  // V: Function that will Initialize Web Socket
  // delay(200);
}

//Init Websocket
void init_WS(){
  webSocket.cleanupClients();
  webSocket.onEvent(on_WebSocket_Event);
  server.addHandler(&webSocket);

  // responde to HTTP_GET requests

        // V: cache control to still reduce the Webpage loading issue.
        // AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/combined.html", "text/html");
        // response->addHeader("Cache-Control", "max-age=3600");  // Cache for 1 hour
        // request->send(response);
        
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    ESP.wdtFeed();
    Serial_Printing_Port.println("\n\n Received HTTP Request. Serving the Webcontents.\n");
    request->send(SPIFFS, "/combined.html", String(), false);
  });
  server.on("/combined.css", HTTP_GET, [](AsyncWebServerRequest *request){
     ESP.wdtFeed();
    request->send(SPIFFS, "/combined.css", "text/css");
  });
  server.on("/fnt/robotoV20LaExLaReg.woff2", HTTP_GET, [](AsyncWebServerRequest *request){
     ESP.wdtFeed();
    request->send(SPIFFS, "/fnt/robotoV20LaExLaReg.woff2", "font/woff2");
  });
  server.on("/Script.js", HTTP_GET, [](AsyncWebServerRequest *request){
     ESP.wdtFeed();
    request->send(SPIFFS, "/Script.js", "text/javascript");  
  });
  server.on("/etLogo.webp", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial_Printing_Port.println(" -->  Serving the image..\n\n");
     ESP.wdtFeed();
    request->send(SPIFFS, "/etLogo.webp", "image/webp");
    delay(1);
  });
 
  server.begin();          //Start server
#ifdef DEBUG_MESSAGES_ACTIVE
  Serial_Printing_Port.println("HTTP server started");
#endif

  // V: keep_alive and client Connected are 0 & false Initially So That's Why it is going to (No client Connected line) Every Time
  keep_alive = 0;
  got_response = false;
  server_running = true;
  client_connected = false;   // V: if true means directly going to WS Timeout(274 Line) and Restarting Wifi
  Serial_Printing_Port.println("Websocket initialisations completed.");
  // queue_WS_MSG(KP_ALIVE);
}

void WS_COM_Handler()
{
  ESP.wdtFeed();
  if(webSocket.availableForWriteAll())          // Space in queue for further messages
  {
    for(uint8_t i = 0; i < ANZ_MSG; ++i)        // Search for the next item to send."
    {
      if(msgPrio.prio[i] == msgPrio.toSend)     // V: this will be set previously in queue_WS_MSG(). that is called in callback functions
      {
        ws_Send_Data((Messages_e)i);            // V: calling the function that will send data to Web-Socket
                                                // V: is used to send a binary message to all clients connected to an AsyncWebSocket
        msgPrio.prio[i] = 0xFF; 
        msgPrio.toSend = ++msgPrio.toSend & MSG_PRIO_LENGTH_MASK;   // V: 
        break;
      }
    }
  }
  ESP.wdtFeed();

  //Send ping to WS 
  if(server_running)
  {
    // V: Cleaning dead client resources for every 5 seconds sinces disconnect event not triggering.
    if( (Ws_clean_count++) >= 70000)
    {
      webSocket.cleanupClients();
    }

    if(client_connected)
    {
      if(got_response)
      {
        if(timer_get_time() - Ws_Ping_Time >= 5000){
          Serial_Printing_Port.printf("\nClients Connected : %d\n",clients_num);
          Ws_Ping_Time = timer_get_time();
          queue_WS_MSG(KP_ALIVE);
          got_response = false;
        }
      }
      else
      {
        if(timer_get_time() - Ws_last_Ping_Time >= 20000){
          Ws_last_Ping_Time = timer_get_time();
          Serial_Printing_Port.println("\n\n---------> No Response. <-----------\n");
          client_connected = false;
          got_response = false;     
          webSocket.cleanupClients(clients_num);
        }
      }
    }
    else
    {
      if(timer_get_time() - lastNoClientMessage > 1500){
        lastNoClientMessage = timer_get_time();
        Serial_Printing_Port.println("   No Client Connected......");
    	 }
    }
  }
  else{
    WiFi.mode(WIFI_OFF);
    delay(10);
    Serial_Printing_Port.println("restart WIFI");
    IPAddress apIP(192, 168, 0, 1);         // V: Again Assiging the IP Address for Restarting.
    Init_Wifi();                            // V: Again Restarting Wi-Fi if Some issue happened.
    // init_WS();
  }
}

void update_AP_password(const char* password){
  #ifdef WIFI_MODE_AP
    if(WiFi.softAPPSK() != (String)password){
      #ifdef DEBUG_MESSAGES_ACTIVE
         Serial_Printing_Port.println("Setting New Password: " + (String)password);
      #endif
      WiFi.softAP(Wifi_string, password, 1,0,1);
    }
    else{
      #ifdef DEBUG_MESSAGES_ACTIVE
         Serial_Printing_Port.println("New password is old Password");
      #endif
    }
 #endif
}

void queue_WS_MSG(Messages_e message)
{
  if(msgPrio.prio[message] == 0xFF)
  {
    msgPrio.prio[message] = msgPrio.sendQueue;
    msgPrio.sendQueue = ++msgPrio.sendQueue & MSG_PRIO_LENGTH_MASK;
  }
}

// Use for debugging purposes only   // V: Not called Anywhere Until Now. They Commented this Function Call Everywhere.
void ws_Send_Text(String data)
{
  while(!webSocket.availableForWriteAll());
  webSocket.textAll(data);
}


/**
 * Event Handler für den Websocket
 * @param	1  Server Objekt
 * @param 2  Client Objekt
 * @param 3  Event Typ
 * @param 4  arg
 * @param 5  Empfangene Daten
 * @param 6  Länge
 * @return	- void
 */
void on_WebSocket_Event(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{ 
  if(type == WS_EVT_CONNECT)
  {
    ESP.wdtFeed();
    keep_alive = 1;
    client_connected = true;
    ++clients_num;  // V: increamenting because a client is connected.
    update_All_Data();
    queue_WS_MSG(KP_ALIVE);
    Ws_Ping_Time = timer_get_time();
    Ws_last_Ping_Time = timer_get_time();
    // delay(3);
    queue_WS_MSG(ALL_DATA);
    source_local = SOURCE_WLAN;
    com_drv.local_source = (uint8_t)source_local;
    COM_SendParameter(LOCAL_SOURCE);
#ifdef DEBUG_MESSAGES_ACTIVE
    Serial_Printing_Port.print("Websocket client connection received ID : ");
    Serial_Printing_Port.print(client->id());
    Serial_Printing_Port.println("");
    clients[client->id()] = client;  // V: keeping track of every client. and adding it to a map 
#endif
  } 
  else if(type == WS_EVT_DISCONNECT)
  {
    ESP.wdtFeed();
    // V: removing client manually
    Serial_Printing_Port.print("Client disconnected ID : ");
    Serial_Printing_Port.print(client->id());
    Serial_Printing_Port.println("");
    clients.erase(client->id());
    client_connected = false;
    source_local = SOURCE_FRONT;
    com_drv.local_source = (uint8_t)source_local;
    COM_SendParameter(LOCAL_SOURCE);
  } 
  else if(type == WS_EVT_DATA)
  {
    if((source_local == SOURCE_FRONT) && (data[0] != KP_ALIVE))
    {
      source_local = SOURCE_WLAN;
      com_drv.local_source = (uint8_t)source_local;
      COM_SendParameter(LOCAL_SOURCE);
      queue_WS_MSG(SOURCE_LOCAL);
    }
    
    handle_Received_Message(data, len);   // V: I think it will handle the message received from the Websocket(Front End)
  }
else
  {
    Serial_Printing_Port.println("Else Event in Websocket event.");
  }
}


/**
 * Callback for websocket messages. Gets called on ws receive (Website --> ESP)
 * @param 1 Pointer to payload
 * @param 2 Length of the data
 * @return void
 */
// V: I think it will handle the message received from the Websocket(Front End)
// V: It will be called by on_WebSocket_Event function
// V: and that will be callled Intially in the init_ws() Function
void handle_Received_Message(uint8_t *data, size_t len)  
{
  ESP.wdtFeed();
  // Serial_Printing_Port.printf("\nData command from Client is : %d\n",data[0]);
  switch(data[0]) // Messages_e
  {
    case SET_V:
      memcpy((uint8_t*)&preset.v, data + 1, sizeof(float));
      com_drv.U_setz = preset.v;
      COM_SendParameter(U_SETZ);
      break;

    case SET_I:
      memcpy((uint8_t*)&preset.i, data + 1, sizeof(float));
      com_drv.I_setz = preset.i;
      COM_SendParameter(I_SETZ);
      break;

    case SET_P:
      memcpy((uint8_t*)&preset.p, data + 1, sizeof(float));
      com_drv.P_setz = preset.p;
      COM_SendParameter(P_SETZ);
      break;

    case SET_R:
      memcpy((uint8_t*)&preset.r, data + 1, sizeof(float));
      com_drv.Ri_setz = preset.r;
      COM_SendParameter(RI_SETZ);
      break;

    case SET_VMPP:
      memcpy((uint8_t*)&preset.vmpp, data + 1, sizeof(float));
      com_drv.Umpp_setz = preset.vmpp;
      COM_SendParameter(UMPP_SETZ);
      break;

    case SET_IMPP:
      memcpy((uint8_t*)&preset.impp, data + 1, sizeof(float));
      com_drv.Impp_setz = preset.impp;
      COM_SendParameter(IMPP_SETZ);
      break;

    case SET_OVP:
      memcpy((uint8_t*)&preset.ovp, data + 1, sizeof(float));
      com_drv.Ovp = preset.ovp;
      COM_SendParameter(OVP);
      break;

    case SET_UVP:
      memcpy((uint8_t*)&preset.uvp, data + 1, sizeof(float));
      if(preset.uvp == -1)
        com_drv.Uvp = Off;
      else
        com_drv.Uvp = preset.uvp;
      COM_SendParameter(UVP);
      break;
    
    case SET_UVPT:
      memcpy((uint8_t*)&preset.uvpT, data + 1, sizeof(float));
      com_drv.T_uvp = preset.uvpT / 1000; // convert it from [ms] to [s]
      COM_SendParameter(T_UVP);
      break;

    case SET_OCP:
      memcpy((uint8_t*)&preset.ocp, data + 1, sizeof(float));
      if(preset.ocp == -1)
        com_drv.Ocp = Off;
      else
        com_drv.Ocp = preset.ocp;
      COM_SendParameter(OCP);
      break;

    case SET_OCPT:
      memcpy((uint8_t*)&preset.ocpT, data + 1, sizeof(float));
      com_drv.T_ocp = preset.ocpT / 1000; // convert it from [ms] to [s]
      COM_SendParameter(T_OCP);
      break;

    case SET_V_SL:
      memcpy((uint8_t*)&preset.vSlope, data + 1, sizeof(float));
      if(preset.vSlope == 0)
        com_drv.U_slope = Off;
      else
        com_drv.U_slope = preset.vSlope;
      COM_SendParameter(U_SLOPE);
      break;

    case SET_I_SL:
      memcpy((uint8_t*)&preset.iSlope, data + 1, sizeof(float));
      if(preset.iSlope == 0)
        com_drv.I_slope = Off;
      else
        com_drv.I_slope = preset.iSlope;
      COM_SendParameter(I_SLOPE);
      break;

    case SET_VLIM:
      memcpy((uint8_t*)&preset.vLim, data + 1, sizeof(float));
      com_drv.U_limit = preset.vLim;
      COM_SendParameter(U_LIMIT);
      break;

    case SET_ILIM:
      memcpy((uint8_t*)&preset.iLim, data + 1, sizeof(float));
      com_drv.I_limit = preset.iLim;
      COM_SendParameter(I_LIMIT);
      break;

    case NT_STATE:
      memcpy((uint8_t*)&status, data + 1, sizeof(Status_t));
      com_drv.Status.Standby = status.power;
      com_drv.Status.Mode = status.mode;
      com_drv.Status.GoToLocal = status.ReqToLocal;
      com_drv.Status.Interlock_Mode = status.interlockMode;
      com_drv.Status.LimMode = status.limMode;
      com_drv.Status.Source = status.source;
      com_drv.Status.Interlock = status.interlock;
      com_drv.Status.LocalLockout = status.LocalLockout;


      if(source_local == WLAN_Source){
        //start On Pending
        if(status.power == ON && intParam.Tenable > 0){
          if(counting){
             counter_active = false;  //deactivate counter
             counting = false;        //stop counting (write On/Off to Start/Stop button)
          }
	      }

        //start On Pending
        if(status.power == OFF && intParam.Tdelay > 0){
          if(counting){
            counter_active = false;  //deactivate counter
            counting = false;        //stop counting (write On/Off to Start/Stop button)
            // ws_Send_Text("ESP (handle status data): cancel counter 1");
          }
	      }
        else if(status.power == OFF && counting){ 
          counter_active = false;
          counting = false;
        }
      }
      COM_SendParameter(NT_STATUS);
      break;

    case DEV_PARAM:
      memcpy((uint8_t*)&devParam.language, data + 1, sizeof(Language_e));
      com_drv.DevParam.Language = devParam.language;
      COM_SendParameter(PARAMETER);

    case REMEMBER:
      memcpy((uint8_t*)&intParam.remember, data + 1, sizeof(uint8_t));
      com_drv.Relase = (bool)intParam.remember;
      COM_SendParameter(RELASE);
      break;

    case TDELAY:
      memcpy((uint8_t*)&intParam.Tdelay, data + 1, sizeof(uint8_t));
      if(intParam.Tdelay == 0)
        com_drv.oDelay = uOff;
      else{
          com_drv.oDelay = intParam.Tdelay;
      } 
      COM_SendParameter(ODELAY);
      break;

    case TENABLE:
      memcpy((uint8_t*)&intParam.Tenable, data + 1, sizeof(uint8_t));
      if(intParam.Tenable == 0)
        com_drv.tEn = uOff;
      else{
        com_drv.tEn = (uint8_t)intParam.Tenable;
      }
      COM_SendParameter(TEN);
      break;

    case LOGGER:
      memcpy((uint8_t*)&intParam.Logger, data + 1, sizeof(uint8_t));
      if(intParam.Logger == 0)
        com_drv.datalogger = uOff;
      else{
        com_drv.datalogger = (uint8_t)intParam.Logger;                                    
      }
      COM_SendParameter(DATALOGGER);
      break;

    case CONTROL_P:
    case CONTROL_RI:
    case CONTROL_PV:
    case MS_STATE:
    case DI_PARAM1:
    case DI_PARAM2:
    case DI_PARAM3:
    case DI_PARAM4:
    case DI_PARAM5:
    case DI_PARAM6:
    case DI_PARAM7:
    case DI_PARAM8:
    case DI_PARAM9:
    case DI_PARAM10:
    case DI_PARAM11:
    case DI_PARAM12:
    case DI_PARAM13:
    case DI_PARAM14:
    case DI_PARAM15:
    case DI_PARAM16:
    case COUNTDOWN_PWR:
    case STATE_SCRIPT:
      break;
      
    case SOURCE_LOCAL:
      memcpy((uint8_t*)&source_local, data + 1, sizeof(uint8_t));
      com_drv.local_source = (uint8_t)source_local;
      COM_SendParameter(LOCAL_SOURCE);
      break;   

    case KP_ALIVE:
      memcpy((uint8_t*)&keep_alive, data + 1, sizeof(uint8_t));
      if(keep_alive == 1){
        got_response = true;
        client_connected = true;
        Ws_last_Ping_Time = timer_get_time();
        Serial_Printing_Port.println("\nPong from client <--\n");
      }
      else if(keep_alive == CLOSE_WS)
      {
        Serial_Printing_Port.println("\n Received closing Request.\n");
        webSocket.cleanupClients(clients_num);
        clients_num--;
        client_connected = false;
      }
      break;
    
    case FOLDBACK_WS:
      memcpy((uint8_t*)&FoldBack.foldback, data +1, sizeof(Foldback_com_e));
      COM_SendParameter(FOLDBACK);
      break;

    case FOLDBACKTM_WS:
      memcpy((uint8_t*)&FoldBack.foldbackTm, data +1, sizeof(uint16));
      COM_SendParameter(FOLDBACKTM);
      break;

    default:
      break;
  }

  // V: Updating set values in all the connected web pages for Synchronisation
  if((clients_num > 1) && ((data[0] >= 2) && (data[0] <= 7)))
  {
    queue_WS_MSG(SET_V);
    queue_WS_MSG(SET_I);
    switch (status.mode)
    {
    case VIP:
    queue_WS_MSG(SET_P);
      break;

    case VIR:
    queue_WS_MSG(SET_R);
      break;
    
    case PVSIM:
    queue_WS_MSG(SET_VMPP);
    queue_WS_MSG(SET_IMPP);
      break;

    default:
      break;
    }
  }
}


// V: This is the Actual Function Sending Information to the Front-End Through Web Sockets
void ws_Send_Data(Messages_e message)
{
  ESP.wdtFeed();
  uint8_t buffer[SEND_BUFFER_SIZE];   // 257 * 4 + 1 = 1029 Bytes
  uint8_t usb_err = 0xE6;

  switch(message)
  {
    case MONVAL:    // V: monitor is a Struct Holding values like V, I, P, R that are Displaying Actually in Touch Display
      buffer[0] = MONVAL;
      memcpy(buffer + 1, (uint8_t*)&monitor, sizeof(Monitor_t));
      webSocket.binaryAll(buffer, sizeof(Monitor_t) + 1);
      break;

    case SET_V:    // V: preset is a Struct holding parameters like vmpp. impp, and many other Including V,I,P,R
      buffer[0] = SET_V;
      memcpy(buffer + 1, (uint8_t*)&preset.v, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_I:  
      buffer[0] = SET_I;
      memcpy(buffer + 1, (uint8_t*)&preset.i, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_P:    
      buffer[0] = SET_P;
      memcpy(buffer + 1, (uint8_t*)&preset.p, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_R:
      buffer[0] = SET_R;
      memcpy(buffer + 1, (uint8_t*)&preset.r, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_VMPP:
      buffer[0] = SET_VMPP;
      memcpy(buffer + 1, (uint8_t*)&preset.vmpp, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_IMPP:
      buffer[0] = SET_IMPP;
      memcpy(buffer + 1, (uint8_t*)&preset.impp, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_OVP:
      buffer[0] = SET_OVP;
      memcpy(buffer + 1, (uint8_t*)&preset.ovp, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_UVP:
      buffer[0] = SET_UVP;
      memcpy(buffer + 1, (uint8_t*)&preset.uvp, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_UVPT:
      buffer[0] = SET_UVPT;
      memcpy(buffer + 1, (uint8_t*)&preset.uvpT, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_OCP:
      buffer[0] = SET_OCP;
      memcpy(buffer + 1, (uint8_t*)&preset.ocp, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_OCPT:
      buffer[0] = SET_OCPT;
      memcpy(buffer + 1, (uint8_t*)&preset.ocpT, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_V_SL:
      buffer[0] = SET_V_SL;
      memcpy(buffer + 1, (uint8_t*)&preset.vSlope, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_I_SL:
      buffer[0] = SET_I_SL;
      memcpy(buffer + 1, (uint8_t*)&preset.iSlope, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_VLIM:
      buffer[0] = SET_VLIM;
      memcpy(buffer + 1, (uint8_t*)&preset.vLim, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case SET_ILIM:
      buffer[0] = SET_ILIM;
      memcpy(buffer + 1, (uint8_t*)&preset.iLim, sizeof(float));
      webSocket.binaryAll(buffer, sizeof(float) + 1);
      break;

    case NT_STATE:   
      buffer[0] = NT_STATE;
      memcpy(buffer + 1, (uint8_t*)&status, sizeof(Status_t));
      webSocket.binaryAll(buffer, sizeof(Status_t) + 1);
      break;

    case CONTROL_P:
      buffer[0] = CONTROL_P;
      memcpy(buffer + 1, (uint8_t*)&control.p, sizeof(Regler_t));
      webSocket.binaryAll(buffer, sizeof(Regler_t) + 1);
      break;
    
    case CONTROL_RI:
      buffer[0] = CONTROL_RI;
      memcpy(buffer + 1, (uint8_t*)&control.ri, sizeof(Regler_t));
      webSocket.binaryAll(buffer, sizeof(Regler_t) + 1);
      break;

    case CONTROL_PV:
      buffer[0] = CONTROL_PV;
      memcpy(buffer + 1, (uint8_t*)&control.pv, sizeof(Regler_t));
      webSocket.binaryAll(buffer, sizeof(Regler_t) + 1);
      break;    

    case XY_AXIS:
      buffer[0] = XY_AXIS;
      memcpy(buffer + 1, (uint8_t*)&xyPlotData.graphParam, sizeof(GraphParam_t));
      webSocket.binaryAll(buffer, sizeof(GraphParam_t) + 1);
      break;

    case XY_MPOS:
      buffer[0] = XY_MPOS;
      memcpy(buffer + 1, (uint8_t*)&xyPlotData.markerPos, sizeof(Point_t));
      webSocket.binaryAll(buffer, sizeof(Point_t) + 1);
      break;
    
    case XY_PDATA:
      buffer[0] = XY_PDATA;
      memcpy(buffer + 1, (uint8_t*)&xyPlotData.plotData, xyPlotData.plotDataSize * sizeof(Point_t));
      webSocket.binaryAll(buffer, (xyPlotData.plotDataSize * sizeof(Point_t)) + 1);
      break;

    case DEV_PARAM:
      buffer[0] = DEV_PARAM;
      memcpy(buffer + 1, (uint8_t*)&devParam, sizeof(DevParam_t));
      webSocket.binaryAll(buffer, sizeof(DevParam_t) + 1);
      break;

    case MS_STATE:
      break;

    case MS_TBL_0:
      buffer[0] = MS_TBL_0;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[0], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_1:
      buffer[0] = MS_TBL_1;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[1], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_2:
      buffer[0] = MS_TBL_2;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[2], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_3:
      buffer[0] = MS_TBL_3;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[3], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_4:
      buffer[0] = MS_TBL_4;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[4], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_5:
      buffer[0] = MS_TBL_5;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[5], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_6:
      buffer[0] = MS_TBL_6;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[6], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_7:
      buffer[0] = MS_TBL_7;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[7], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_8:
      buffer[0] = MS_TBL_8;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[8], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_9:
      buffer[0] = MS_TBL_9;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[9], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_10:
      buffer[0] = MS_TBL_10;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[10], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_11:
      buffer[0] = MS_TBL_11;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[11], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_12:
      buffer[0] = MS_TBL_12;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[12], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_13:
      buffer[0] = MS_TBL_13;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[13], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_14:
      buffer[0] = MS_TBL_14;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[14], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case MS_TBL_15:
      buffer[0] = MS_TBL_15;
      memcpy(buffer + 1, (uint8_t*)&NtParam.msmonitor[15], sizeof(NtMonitor_t) - 4);
      webSocket.binaryAll(buffer, sizeof(NtMonitor_t) + 1 - 4);
      break;

    case SW_ID:
      buffer[0] = SW_ID;
      memcpy(buffer + 1, (uint8_t*)&ident, sizeof(Ident_t));
      webSocket.binaryAll(buffer, sizeof(Ident_t) + 1);
      break;
    
    case CompID:
      // buffer[0] = CompID;
      // memcpy(buffer + 1, (Compid_t*)&compid, sizeof(Compid_t));
      // webSocket.binaryAll(buffer, sizeof(T_Compid) + 1);
      break; 

     case REMEMBER:
      buffer[0] = REMEMBER;
      memcpy(buffer + 1, (uint8_t*)&intParam.remember, sizeof(uint8_t));
      webSocket.binaryAll(buffer, sizeof(uint32_t) + 1);
      break;
    
     case TDELAY:
      buffer[0] = TDELAY;
      memcpy(buffer + 1, (uint8_t*)&intParam.Tdelay, sizeof(uint8_t));
      webSocket.binaryAll(buffer, sizeof(uint32_t) + 1);
      break;
    
     case TENABLE:
      buffer[0] = TENABLE;
      memcpy(buffer + 1, (uint8_t*)&intParam.Tenable, sizeof(uint8_t));
      webSocket.binaryAll(buffer, sizeof(uint32_t) + 1);
      break;
    
     case LOGGER:
      buffer[0] = LOGGER;
      memcpy(buffer + 1, (uint8_t*)&intParam.Logger, sizeof(uint8_t));
      webSocket.binaryAll(buffer, sizeof(uint32_t) + 1);
      break;

     case ERR_MSG:
      buffer[0] = ERR_MSG;
      memcpy(buffer + 1, (uint8_t*)&NtParam.errmsg, sizeof(ErrMessage_t));
      webSocket.binaryAll(buffer, sizeof(ErrMessage_t) + 1);
      break;

      case DI_PARAM1:
        buffer[0] = DI_PARAM1;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[0], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM2:
        buffer[0] = DI_PARAM2;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[1], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM3:  
        buffer[0] = DI_PARAM3;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[2], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM4:
        buffer[0] = DI_PARAM4;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[3], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;

      case DI_PARAM5:
        buffer[0] = DI_PARAM5;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[4], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM6:
        buffer[0] = DI_PARAM6;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[5], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM7:
        buffer[0] = DI_PARAM7;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[6], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM8:
        buffer[0] = DI_PARAM8;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[7], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;

      case DI_PARAM9:
        buffer[0] = DI_PARAM9;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[8], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM10:
        buffer[0] = DI_PARAM10;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[9], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM11:
        buffer[0] = DI_PARAM11;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[10], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM12:
        buffer[0] = DI_PARAM12;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[11], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;

      case DI_PARAM13:
        buffer[0] = DI_PARAM13;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[12], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM14:
        buffer[0] = DI_PARAM14;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[13], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM15:
        buffer[0] = DI_PARAM15;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[14], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case DI_PARAM16:
        buffer[0] = DI_PARAM16;
        memcpy(buffer + 1, (uint8_t*)&NtParam.interface[15], sizeof(Iface_t));
        webSocket.binaryAll(buffer, sizeof(Iface_t) + 1);
        break;
      
      case COUNTDOWN_PWR:
        buffer[0] = COUNTDOWN_PWR;
        memcpy(buffer + 1, (uint8_t*)&count_pwr, sizeof(uint8_t));
        // ws_Send_Text("ESP (ws_Send_Data(COUNTDOWN_PWr): Send power countdown = " + (String)count_pwr);
        webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
        break;

      case USB_ACTIVE:
        buffer[0] = USB_ACTIVE;
        memcpy(buffer + 1, (uint8_t*)&usb_active, sizeof(uint8_t));
        webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
        break;

      case STATE_SCRIPT:
        buffer[0] = STATE_SCRIPT;
        memcpy(buffer + 1, (uint8_t*)&state_script, sizeof(uint8_t));
        webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
        break;
      
      case COMMAND_SCRIPT:
        buffer[0] = COMMAND_SCRIPT;
        memcpy(buffer + 1, (uint8_t*)&cmd_script, sizeof(Script_command_t));
        webSocket.binaryAll(buffer, sizeof(Script_command_t) + 1);
        break;

      case USB_ERR:
        buffer[0] = USB_ERR;
        memcpy(buffer + 1, (uint8_t*)&usb_err, sizeof(uint8_t));
        webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
        break;

      case SOURCE_LOCAL:
        buffer[0] = SOURCE_LOCAL;
        memcpy(buffer + 1, (uint8_t*)&source_local, sizeof(uint8_t));
        webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
        break;

      case COUNTER:
        buffer[0] = COUNTER;
        memcpy(buffer + 1, (uint8_t*)&counter, sizeof(uint8_t));
        webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
        break;

      case COUNTER_ACTIVE:
        buffer[0] = COUNTER_ACTIVE;
        memcpy(buffer + 1, (uint8_t*)&counter_active, sizeof(uint8_t));
        webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
        break;

      case ALL_DATA:
      {
        // ws_Send_Text("ESP (send ALL_DATA)");
        uint16_t n = 0;
        buffer[n] = ALL_DATA;
        ++n;

        memcpy(buffer + n, (uint8_t*)&monitor, sizeof(Monitor_t));                    // 	V: 16 bytes
        n += sizeof(Monitor_t); 

        memcpy(buffer + n, (uint8_t*)&preset, sizeof(Preset_t));                     // 	V: 60 bytes
        n += sizeof(Preset_t);

        memcpy(buffer + n, (uint8_t*)&control, sizeof(Control_t));                   // 	V: 36 bytes
        n += sizeof(Control_t);

        memcpy(buffer + n, (uint8_t*)&status, sizeof(Status_t));                     // 	V: 8 bytes
        n += sizeof(Status_t);

        memcpy(buffer + n, (uint8_t*)&xyPlotData.graphParam, sizeof(GraphParam_t));   // V: 24 bytes
        n += sizeof(GraphParam_t);

        memcpy(buffer + n, (uint8_t*)&xyPlotData.markerPos, sizeof(Point_t));         // V: 4 bytes
        n += sizeof(Point_t);

        memcpy(buffer + n, (uint8_t*)&devParam, sizeof(DevParam_t));                  // V: 28 bytes
        n += sizeof(DevParam_t);

        memcpy(buffer + n, (uint8_t*)&NtParam.msconfig, sizeof(MasterSlave_t));       // V: 4 bytes
        n += sizeof(MasterSlave_t);

        memcpy(buffer + n, (uint8_t*)&ms_available, sizeof(uint8_t));                 // V: 1 byte
        n += sizeof(uint8_t);

        memcpy(buffer + n, (uint8_t*)&ident, sizeof(Ident_t));                        // V: 2 bytes
        n += sizeof(Ident_t);

        memcpy(buffer + n, (uint8_t*)&compid, sizeof(Compid_t));                      // V: 256 bytes
        n += sizeof(Compid_t);

        memcpy(buffer + n, (uint8_t*)&intParam, sizeof(IntParam_t));                  // V: 4 bytes.
        n += sizeof(IntParam_t);

        memcpy(buffer + n, (uint8_t*)&usb_active, sizeof(uint8_t));
        n += sizeof(uint8_t);

        memcpy(buffer + n, (uint8_t*)&source_local, sizeof(uint8_t));
        n += sizeof(uint8_t);

        memcpy(buffer + n, (uint8_t*)&count_pwr, sizeof(uint8_t));
        n += sizeof(uint8_t);

        webSocket.binaryAll(buffer, n);
      }
      break;

    case KP_ALIVE:
      // ws_Send_Text("Ping");
      buffer[0] = KP_ALIVE;
      if(STM_Freezed)
      {
        keep_alive = (uint8)STM_FREEZED;
        Time_STM_Last_msg_received = timer_get_time();
        STM_Freezed = false;
      }
      memcpy(buffer + 1, (uint8_t*)&keep_alive, sizeof(uint8_t));
      webSocket.binaryAll(buffer, sizeof(uint8_t) + 1);
      Serial_Printing_Port.println("Ping to client --> \n");
      break;

    case FOLDBACK_WS:
    buffer[0] = FOLDBACK_WS;
    memcpy(buffer +1, (uint8_t*)&FoldBack.foldback, sizeof(Foldback_com_e));
    webSocket.binaryAll(buffer,sizeof(Foldback_com_e) + 1);
      break;
	  
    case FOLDBACKTM_WS:
    buffer[0] = FOLDBACKTM_WS;
    memcpy(buffer + 1, (uint8_t*)&FoldBack.foldbackTm, sizeof(uint16));
    webSocket.binaryAll(buffer,sizeof(uint16) + 1);
      break;

    default:
      break;
  };
}

//handle counter, if active
void Counter_Handler(){
  if(source_local == WLAN_Source){
    if(counter_active){
      if(timer_get_time() - pendtimer > 1000){  //trigger every 1000ms (1s)
        // counting = true; //enable counting
        count_pwr--;
        com_drv.pwrCountdown = count_pwr;
        // ws_Send_Text("ESP (Counter_Handler): Counting: " + (String)count_pwr);
        if(count_pwr > 0){
          // ws_Send_Text("ESP (Counter_Handler): send power countdown (counter >0): " + (String)count_pwr);
          queue_WS_MSG(COUNTDOWN_PWR);
          COM_SendParameter(PWR_COUNTDOWN);
        }
        else{
          // ws_Send_Text("ESP (Counter_Handler): stopping power countdown");
          counter_active = false; //disable counter
          if(status.power == OFF)
            status.power = ON;
          else
            status.power = OFF;

          switch(status.power){
            case ON:
                if(intParam.Tenable > 0){ //elapsing with t enable
                  // ws_Send_Text("ESP (Counter_Handler): counter elapsed, next mode ON_PEND " + (String)status.power);
                  count_pwr = intParam.Tenable;
                  com_drv.pwrCountdown = count_pwr;
                  start_counter();
                }
                else{
                  // ws_Send_Text("ESP (Counter_Handler): counter elapsed, next mode ON");
                } 

              break;

            case OFF:
              // ws_Send_Text("ESP (Counter_Handler): counter elapsed, next mode OFF");
              counting = false;
              break;

            default:
              break;
          }
          queue_WS_MSG(COUNTDOWN_PWR);
          // ws_Send_Text("ESP (Counter_Handler): send power countdown (counter = 0): " + (String)count_pwr);
          COM_SendParameter(PWR_COUNTDOWN);

          queue_WS_MSG(NT_STATE);

          com_drv.Status.Standby = status.power;
          // update_xy(GRAPH_SEND_ALL);
          COM_SendParameter(NT_STATUS);
        }
        pendtimer = timer_get_time();
      }
    }
    else{
      if(timer_get_time() - pendtimer > 1000){
            // ws_Send_Text("ESP (Counter_Handler): counter stopped");
#ifdef DEBUG_MESSAGES_ACTIVE
        // Serial_Printing_Port.println("Counter stopped");
        // queue_WS_MSG(COUNTDOWN_PWR);
#endif
        if(count_pwr != 0){
          count_pwr = 0;
          queue_WS_MSG(COUNTDOWN_PWR);
          queue_WS_MSG(NT_STATE);
          com_drv.pwrCountdown = count_pwr;
          // ws_Send_Text("ESP (Counter_Handler): send power countdown (counter stopped): " + (String)count_pwr);
          COM_SendParameter(PWR_COUNTDOWN);
          COM_SendParameter(NT_STATUS);
        }
        pendtimer = timer_get_time();
      }
    }
  }
}

//send power countdown to WS and STM
void start_counter(){
  // count_pwr = countdown;
	counter_active = true; //activate timer
	pendtimer = timer_get_time();
  // ws_Send_Text("ESP (start_counter): count_pwr = " + (String)count_pwr);
}

void update_All_Data()
{
  Clean_serial_buffer();
  COM_GetParameter(COMP_ID); 
	COM_GetParameter(NT_STATUS);
	COM_GetParameter(PARAMETER);
	COM_GetParameter(U_SETZ);
	COM_GetParameter(I_SETZ);
	COM_GetParameter(RI_SETZ);
	COM_GetParameter(P_SETZ);
	COM_GetParameter(IMPP_SETZ);	
	COM_GetParameter(UMPP_SETZ);
	COM_GetParameter(I_LIMIT);
	COM_GetParameter(U_LIMIT);
  COM_GetParameter(U_SLOPE);
  COM_GetParameter(I_SLOPE);
  COM_GetParameter(TEN);
  COM_GetParameter(RELASE);
  COM_GetParameter(ODELAY);
  COM_GetParameter(DATALOGGER);
  COM_GetParameter(OVP);
  COM_GetParameter(UVP);
  COM_GetParameter(OCP);
  COM_GetParameter(T_OCP);
  COM_GetParameter(T_UVP);
  COM_GetParameter(FOLDBACK);
  COM_GetParameter(FOLDBACKTM);
  ESP.wdtFeed();
}

void get_All_WS_data(){
  for(uint16_t i = 1; i<ANZ_MSG; ++i){
    if(i == 74){
      break;
    }
    if(i != 72 && i != 71){ //request script && usb_err
       queue_WS_MSG((Messages_e)i);
       delay(5);
    }
   
  }
}

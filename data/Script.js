/*
*   Date :- 30.01.25
*   Day  :- Thursday.
*   
*   Author  :- Vijay Kumar Setti (ET System India)
*   Role    :- Embedded Firmware Engineer.
*   Purpose :- Already this version of code is Released but minified accidentally(Minified version is not understandable)
*              So, for better understanding and with better approaches, I am creating new file
*              With same old features & better optimised code. New features can be added going forward.
*/

// ------------------- Variable Declarations ------------

const WEBSITE_VERSION = "V009";
var function_id, function_id2,              // V: Variables to store registered function id's for setinterval Thread.
user_in_local_src,
ws = null,                                  // V: Websocket variable
nopong_cnt = 0,                             // V: These 2 variables are used to keep the Webcocket communication Persistant.
got_response = 0;

var is_onboard = true;   // V: true  -> Onboard data flow will be continous
                          //    false -> Dev-kit data will not flow.


var user_in_set = Object.assign({}, set);           // V: Variable to store Preset values from user in Web-Interface
var user_in_status = Object.assign({}, state);      // V: Variable to store Status struct values from user in Web-Interface
var data_flow = 0;                                  // V: Variable to monitor the data flow b/w Server & Web-Interface
let set_arr = ["v", "i", "p", "r", "vmpp", "impp"]; // V: currently not in use.

// V: Hardware limits Structure some default limits are giving initially.
var DevParams = { umax: 700, imax: 70, pmax: 49000, rmax: 160, rmin: -160 };

// V: Preset values Structure
var set = {
    v: 0,
    i: 0,
    p: 0,
    r: 0,
    vmpp: 0,
    impp: 0,
    ovp: 0,
    uvp: 0,
    uvpT: 0,
    ocp: 0,
    ocpT: 0,
    vSlope: 0,
    iSlope: 0,
    vLim: 0,
    iLim: 0,
};

  // V: PID controller values structure(Not used currently)
var control = {
    p: { p: 1, i: 1, d: 1 },
    ri: { p: 1, i: 1, d: 1 },
    pv: { p: 1, i: 1, d: 1 },
  };

  // V: Accuracy structure(Not used currently)
var accuracy = { v: 0.005, i: 0.001, p: 0.001, r: 0.001 };
// V: Monitor values structre
var monitor = { v: 0, i: 0, p: 0, r: 0 };
// V: Init param's or config page values strut
var configuration_params = {RemLastsettings:0, Tdelay:0, TEnable:0, DataLogging:0,};
var Vslope_max = 1, Islope_max = 1, TEnable_max = 100, OutputOnDelay_max = 100, Ovp_max = 0, Ocp_max = 0,
    DataLogging_max = 100, Uvp_max = 0, Uvp_min = 0, UvpT_min = 100, UvpT_max = 1000, OcpT_min = 1, OcpT_max = 5000,
    FoldbackTm_min = 10, FoldbackTm_max = 10000;

// ------------------- ENUM Declarations ------------
// V: Enum used to declrae which command(message) is received.
const Messages_e = {
    NONE: 0,
    MONVAL: 1,
    SET_V: 2,
    SET_I: 3,
    SET_P: 4,
    SET_R: 5,
    SET_VMPP: 6,
    SET_IMPP: 7,
    SET_OVP: 8,
    SET_UVP: 9,
    SET_UVPT: 10,
    SET_OCP: 11,
    SET_OCPT: 12,
    SET_V_SL: 13,
    SET_I_SL: 14,
    SET_VLIM: 15,
    SET_ILIM: 16,
    NT_STATE: 17,
    CONTROL_P: 18,
    CONTROL_RI: 19,
    CONTROL_PV: 20,
    XY_AXIS: 21,
    XY_MPOS: 22,
    XY_PDATA: 23,
    MS_SETTINGS: 24,
    MS_TBL_0: 25,
    MS_TBL_1: 26,
    MS_TBL_2: 27,
    MS_TBL_3: 28,
    MS_TBL_4: 29,
    MS_TBL_5: 30,
    MS_TBL_6: 31,
    MS_TBL_7: 32,
    MS_TBL_8: 33,
    MS_TBL_9: 34,
    MS_TBL_10: 35,
    MS_TBL_11: 36,
    MS_TBL_12: 37,
    MS_TBL_13: 38,
    MS_TBL_14: 39,
    MS_TBL_15: 40,
    MS_STATE: 41,
    MS_AVAILABLE: 42,
    DEVPARAM: 43,
    SW_ID: 44,
    CompID: 45,
    REMEMBER: 46,
    TDELAY: 47,
    TENABLE: 48,
    LOGGER: 49,
    ERR_MSG: 50,
    DI_PARAM1: 51,
    DI_PARAM2: 52,
    DI_PARAM3: 53,
    DI_PARAM4: 54,
    DI_PARAM5: 55,
    DI_PARAM6: 56,
    DI_PARAM7: 57,
    DI_PARAM8: 58,
    DI_PARAM9: 59,
    DI_PARAM10: 60,
    DI_PARAM11: 61,
    DI_PARAM12: 62,
    DI_PARAM13: 63,
    DI_PARAM14: 64,
    DI_PARAM15: 65,
    DI_PARAM16: 66,
    COUNTDOWN_PWR: 67,
    USB_ACTIVE: 68,
    STATE_SCRIPT: 69,
    COMMAND_SCRIPT: 70,
    USB_ERR: 71,
    REQUEST_SCRIPT: 72,
    SOURCE_LOCAL: 73,
    COUNTER: 74,
    COUNTER_ACTIVE: 75,
    ALL_DATA: 76,
    KP_ALIVE: 77,
    FOLDBACK : 78,
    FOLDBACKTM : 79,
    ANZ_MSG: 80,
  };

const Websocket_com_e = {DEAD:0, ALIVE:1, STM_FREEZED:2, CLOSE_WS:3};
const Last_Settings_e = {OFF:0, ON:1};
const FoldBack_e = {Fold_OFF:0, Fold_CV:1, Fold_CC:2, Fold_CP:3};
const Power_e = { ON: 0, OFF: 1, OFF_PEND: 2, ON_PEND: 3 };                 // V: Enum to describe the power statem device
const Mode_e = { VC: 0, VCP: 1, VCR: 2, PVsim: 3, User: 4, Script: 5 };     // V: Enum to describe the mode of device
const Local_Source_e = { Local: 0, WLAN: 1 };                               // V: Enum to describe the control of device(Ambiguties currently)
const Source_e = { Front: 0, AI: 1, Remote: 2 };                            // V: Ambiguties currently(Not in use currently.)
const Interlock_e = { lowAct: 0, highAct: 1 };                              // V: currenlt dont have the idea.
const LimMode_e = { CV: 0, CC: 1, CP: 2 };                                  // V: Enum to describe the CV/CC/CP of the device
const Fault_e = { NA: -1, Ovp: 0, Ocp: 1, Uvp: 2, Otp: 3 };                 // V: Enum to describe the Fault of the device
const Page_e = { NA_e: -1, Index: 0, Control: 1, Webconfig: 2, Display: 3, Protection : 4, Configuration : 5 } // V: Enum used for Differentiating b/w pages not used anymore.
const set_obj_str = ["ua", "ia", "pa", "ra", "umpp", "impp", "Ovp", "Uvp", "UvpT", "Ocp", "OcpT", "Vslope", "Islope", "Vlim", "Ilim"];   // V: Enum used for Differentiating b/w the set values
const Configuration_Page_strs = ["Vlim", "Ilim", "Vslope", "Islope", "TEnable", "OutputOnDelay", "DataLogging", "RemLastSettings"];
const Protection_Page_strs = ["Ovp", "Uvp", "Ocp", "UvpT", "OcpT", "Foldback"];


// ------------------- Variables that uses the above Const Definitions ------------

var local_src = Local_Source_e.WLAN;     // V: Variable to store Source/control input from user in Web-Interface
var identify_page = Page_e.NA_e

// V: Status Structre
var state = {
  power: Power_e.OFF,
  mode: Mode_e.VC,
  GotoLocal: 0,
  intLockMode: Interlock_e.lowAct,
  limMode: LimMode_e.CV,
  source: Source_e.Front,
  interlock: 0,
  LocalLockout: 0,
  fault: Fault_e.NA,
};

var FoldBack = {
  Foldback : FoldBack_e.OFF,
  FoldbackTm : 0,
};

// ------------------- Private functions Definitions ------------

// V: Starting function that will called in when the window is loaded.
window.onload = setup;

// V: Adding a event listener when webpage is closing so that 
window.addEventListener("beforeunload", function() {
    closeWebSocket();
    console.log("Event listener before closing the page.");
});

// V: function to close the websocket 
function closeWebSocket() {
     send_keepAlive(Websocket_com_e.CLOSE_WS);
      ws.close();
}

// V: starting function to be called after page loading
function setup() 
{
  switch_page('Index');
    if(is_onboard)
    {
        ws_Connect();
    }
}

// V: This will initialize & maintain the websocket communication b/w webpage & server
function ws_Connect() 
{
  ws = new WebSocket("ws://" + window.location.hostname + "/ws");
  console.log("connecting to Websocket...");
  ws.binaryType = "arraybuffer";
  ws.onopen = function () {
  console.log("WebSocket connected.");
  function_id = setInterval(ws_PingPong, 5000);
  send_keepAlive(Websocket_com_e.ALIVE);
  };

  ws.onclose = function () {
    console.log("Socket is getting closed.");
    send_keepAlive(Websocket_com_e.CLOSE_WS);
    ws.close();
  };

  ws.onmessage = function (data) {
    if (typeof data.data === "string") {
      console.log(data.data);
    }
    else {
      handle_WS_Message(data);
    }
  };

  ws.onerror = function () {
    showError("Error in WLAN communication \nPlease check if WLAN is Connected.");
    ws.close();
  };
}

// V: function that will be called for every 5 seconds to check the state of Websocket.
function ws_PingPong() {
    if (got_response) 
    {
      send_keepAlive(Websocket_com_e.ALIVE);
      got_response = false;
      nopong_cnt = 0;
    } else 
    {
      nopong_cnt++;
      if (nopong_cnt >= 3)
      {
        clearInterval(function_id);
        showError("WLAN maybe Disconnected for a moment.\n Please Refresh the page.");
        send_keepAlive(Websocket_com_e.CLOSE_WS);
        ws.close();
        // ws_Connect();
      } 
      else 
      {
        send_keepAlive(Websocket_com_e.ALIVE);
        got_response = false;
      }
    }
  }

// V: function that will recive messages from websocket and call respective functions
function handle_WS_Message(e) 
{
    var t = new Uint8Array(e.data);
    console.log(t[0]);
  
    switch (t[0]) {
        // V: Monitor values structure 
      case Messages_e.MONVAL:
        data_flow = true;
        update_monitor_values(new Float32Array(e.data.slice(1, 17)));
        break;

        // V: Status Structre that will hold device info
      case Messages_e.NT_STATE:
        Update_status(t.slice(1, 10));
        break;
        
        // V: Preset Voltage value
      case Messages_e.SET_V:
        set.v = new Float32Array(e.data.slice(1, 5))[0];
        set.v = parseFloat(set.v.toFixed(2));
        update_set_values(set.v, set_obj_str[0]);
        break;

        // V: Preset Current value
      case Messages_e.SET_I:
        set.i = new Float32Array(e.data.slice(1, 5))[0];
        set.i = parseFloat(set.i.toFixed(2));
        update_set_values(set.i, set_obj_str[1]);
        break;
  
        // V: Preset Power value
      case Messages_e.SET_P:
        set.p = new Float32Array(e.data.slice(1, 5))[0];
        set.p = parseFloat(set.p.toFixed(2));
        update_set_values(set.p, set_obj_str[2]);
        break;

        // V: Preset Resistance value
      case Messages_e.SET_R:
        set.r = new Float32Array(e.data.slice(1, 5))[0];
        set.r = parseFloat(set.r.toFixed(2));
        update_set_values(set.r, set_obj_str[3]);
        break;
  
        // V: Preset Vmpp value
      case Messages_e.SET_VMPP:
        set.vmpp = new Float32Array(e.data.slice(1, 5))[0];
        set.vmpp = parseFloat(set.vmpp.toFixed(2));
        update_set_values(set.vmpp, set_obj_str[4]);
        break;
  
        // V: Preset Impp value
      case Messages_e.SET_IMPP:
        set.impp = new Float32Array(e.data.slice(1, 5))[0];
        set.impp = parseFloat(set.impp.toFixed(2));
        update_set_values(set.impp, set_obj_str[5]);
        break;
  
        // V: Device Programmable Voltage Limit
      case Messages_e.SET_VLIM:
        set.vLim = new Float32Array(e.data.slice(1, 5))[0];
        set.vLim = parseFloat(set.vLim.toFixed(2));
        update_configuration_page_values("Vlim",set.vLim);
        break;

        // V: Device Programmable Current Limit
      case Messages_e.SET_ILIM:
        set.iLim = new Float32Array(e.data.slice(1, 5))[0];
        set.iLim = parseFloat(set.iLim.toFixed(2));
        update_configuration_page_values("Clim",set.iLim);
        break;

      case Messages_e.SET_V_SL:
        set.vSlope = new Float32Array(e.data.slice(1, 5))[0];
        set.vSlope = parseFloat(set.vSlope.toFixed(0));
        update_configuration_page_values("VSlope",set.vSlope);
        break;

      case Messages_e.SET_I_SL:
        set.iSlope = new Float32Array(e.data.slice(1, 5))[0];
        set.iSlope = parseFloat(set.iSlope.toFixed(0));
        update_configuration_page_values("ISlope",set.iSlope);
        break;

      case Messages_e.SET_OVP:
        set.ovp = new Float32Array(e.data.slice(1, 5))[0];
        set.ovp = parseFloat(set.ovp.toFixed(2));
        update_Protection_page_values("Ovp",set.ovp);
        break;

      case Messages_e.SET_UVP:
        set.uvp = new Float32Array(e.data.slice(1, 5))[0];
        set.uvp = parseFloat(set.uvp.toFixed(2));
        update_Protection_page_values("Uvp",set.uvp);
        break;

      case Messages_e.SET_UVPT:
        set.uvpT = new Float32Array(e.data.slice(1, 5))[0];
        update_Protection_page_values("UvpT",set.uvpT);
        break;

      case Messages_e.SET_OCP:
        set.ocp = new Float32Array(e.data.slice(1, 5))[0];
        set.ocp = parseFloat(set.ocp.toFixed(2));
        update_Protection_page_values("Ocp",set.ocp);
        break;

      case Messages_e.SET_OCPT:
        set.ocpT = new Float32Array(e.data.slice(1, 5))[0];
        update_Protection_page_values("OcpT",set.ocpT);
        break;

       // V: Device Hardware limits Can't change
       case Messages_e.DEVPARAM:
        var s = new Float32Array(e.data.slice(9, 29));
        update_dev_params(s);
        break; 
  
        // V: Source of the Front. has some ambiguties.
      case Messages_e.SOURCE_LOCAL:
        user_in_local_src = local_src = new Uint8Array(e.data.slice(1, 2))[0];
        update_source(local_src);
        break;

        // V: These down 4 values are coming as 32 bit integers, update to 8 bit if any ambiguties found.
      case Messages_e.REMEMBER:
        configuration_params.RemLastsettings = new Uint8Array(e.data.slice(1, 2))[0];
        update_configuration_page_values("RemLastSettings",configuration_params.RemLastsettings);
        break;

      case Messages_e.TDELAY:
        configuration_params.Tdelay = new Uint8Array(e.data.slice(1, 2))[0];
        update_configuration_page_values("OutputOnDelay",configuration_params.Tdelay);
        break;

      case Messages_e.TENABLE:
        configuration_params.TEnable = new Uint8Array(e.data.slice(1, 2))[0];
        update_configuration_page_values("TEnable",configuration_params.TEnable);
        break;

      case Messages_e.LOGGER:
        configuration_params.DataLogging = new Uint8Array(e.data.slice(1, 2))[0];
        update_configuration_page_values("DataLogging",configuration_params.DataLogging);
        break;
      
      case Messages_e.FOLDBACK:
        FoldBack.Foldback = new Uint8Array(e.data.slice(1,5))[0];
        update_Protection_page_values("Foldback",FoldBack.Foldback);
        break;

      case Messages_e.FOLDBACKTM:
        FoldBack.FoldbackTm = new Float32Array(e.data.slice(1,5))[0];
        update_Protection_page_values("FoldbakTm",FoldBack.FoldbackTm);
        break;

      case Messages_e.XY_AXIS:
      case Messages_e.XY_MPOS:
      case Messages_e.XY_PDATA:
      case Messages_e.CONTROL_P:
      case Messages_e.CONTROL_RI:
      case Messages_e.CONTROL_PV:
      case Messages_e.MS_TBL_0:
      case Messages_e.MS_TBL_1:
      case Messages_e.MS_TBL_2:
      case Messages_e.MS_TBL_3:
      case Messages_e.MS_TBL_4:
      case Messages_e.MS_TBL_5:
      case Messages_e.MS_TBL_6:
      case Messages_e.MS_TBL_7:
      case Messages_e.MS_TBL_8:
      case Messages_e.MS_TBL_9:
      case Messages_e.MS_TBL_10:
      case Messages_e.MS_TBL_11:
      case Messages_e.MS_TBL_12:
      case Messages_e.MS_TBL_13:
      case Messages_e.MS_TBL_14:
      case Messages_e.MS_TBL_15:
      case Messages_e.MS_STATE:
      case Messages_e.SW_ID:
      case Messages_e.CompID:
      case Messages_e.ERR_MSG:
      case Messages_e.DI_PARAM1:
      case Messages_e.DI_PARAM2:
      case Messages_e.DI_PARAM3:
      case Messages_e.DI_PARAM4:
      case Messages_e.DI_PARAM5:
      case Messages_e.DI_PARAM6:
      case Messages_e.DI_PARAM7:
      case Messages_e.DI_PARAM8:
      case Messages_e.DI_PARAM9:
      case Messages_e.DI_PARAM10:
      case Messages_e.DI_PARAM11:
      case Messages_e.DI_PARAM12:
      case Messages_e.DI_PARAM13:
      case Messages_e.DI_PARAM14:
      case Messages_e.DI_PARAM15:
      case Messages_e.DI_PARAM16:
      case Messages_e.COUNTDOWN_PWR:
      case Messages_e.USB_ACTIVE:
      case Messages_e.STATE_SCRIPT:
      case Messages_e.COMMAND_SCRIPT:
      case Messages_e.USB_ERR:
      case Messages_e.COUNTER:
      case Messages_e.COUNTER_ACTIVE:
        break;

        // V: This contains all data of the Device will sent for every new connection(Websocket)
      case Messages_e.ALL_DATA:
        // V: All data buffer bytes in struct wise(these down 4 are the struct's we mainly use)
			  // 1st  -> monitor struct(4 float mem's) 					             = 16 bytes
			  // 2nd  -> preset struct(15 float mem's) 					             = 60 bytes
			  // 3rd  -> control struct(3 inner structs of 3 floats each)	   = 36 bytes
			  // 4th  -> status struct(8 uint's) 							               = 9 bytes
        // 5th  -> Graph param's struct(with internal struct's)        = 24 bytes
        // 6th  -> Graph points struct                                 = 4 bytes
        // 7th  -> Device parameters struct                            = 28 bytes
        // 8th  -> Master slave table struct                           = 4 bytes
        // 9th  -> MS avaialable byte                                  = 1 byte
        // 10th -> Ident struct                                        = 2 bytes
        // 11th -> Kompid struct                                       = 256 bytes
        // 12th -> InitParam's struct(Remember,Tdelay,Tenable,logging)   = 4 bytes

        update_monitor_values(new Float32Array(e.data.slice(1, 17)));
        var dev_lims = new Float32Array(e.data.slice(153, 173));
        update_dev_params(dev_lims);
        Update_All_Set_values_First_Time(new Float32Array(e.data.slice(17, 77)));
        Update_status(new Uint8Array(e.data.slice(113, 122)));
        update_all_Protection_page_values_first_time(new Uint8Array(e.data.slice(441, 445)));
        update_all_configuration_page_values_first_time();
        break;

        // V: Keep alive case to keep Server & Client communication persistant. 
      case Messages_e.KP_ALIVE:
        switch(t[1])
        {
          case Websocket_com_e.DEAD:
            showError("Received Dead from server.");
            break;
          
          case Websocket_com_e.ALIVE:
            console.log("Ping from Server <-- ");
            got_response = true;
            break;

          case Websocket_com_e.STM_FREEZED:
            showError("Display is freezed.\n Please Reset the Power Supply.");
            break;

          case Websocket_com_e.CLOSE_WS:
            showError("Received Websocket Closing request From server.");
            break; 
        }
        break;
      
      default:
        break;
    }
  }

// V: function that will update monitor values in the webpages
function update_monitor_values(recv_monitor_val) 
{
  if (recv_monitor_val) {
    arr_To_Obj(monitor, recv_monitor_val);
  }

  monitor.v = parseFloat(monitor.v.toFixed(2));
  monitor.i = parseFloat(monitor.i.toFixed(2));
  monitor.p = parseFloat(monitor.p.toFixed(2));
  monitor.r = parseFloat(monitor.r.toFixed(2));

  // V: updating the new value in all pages.
  document.querySelector(".uText").textContent = monitor.v;
  document.querySelector(".iText").textContent = monitor.i;
  document.querySelector(".pText").textContent = monitor.p;
  document.querySelector(".rText").textContent = monitor.r;
  document.querySelector(".mon_uText").textContent = monitor.v;
  document.querySelector(".mon_iText").textContent = monitor.i;
  document.querySelector(".mon_pText").textContent = monitor.p;
  document.querySelector(".mon_rText").textContent = monitor.r;
}

// V: this function will be called from the ALL_DATA case 
// V: this is used to call the update_set_values() independently for every present set value
function Update_All_Set_values_First_Time(recv_preset_val)
{
	if(recv_preset_val)
	{
		arr_To_Obj(set,recv_preset_val);
	}

	// V: conevrting all float values into values that will appear 2 numbers after decimal point
	set.v = parseFloat(set.v.toFixed(2));
	set.i = parseFloat(set.i.toFixed(2));
	set.p = parseFloat(set.p.toFixed(2));
	set.r = parseFloat(set.r.toFixed(2));
	set.impp = parseFloat(set.impp.toFixed(2));
	set.vmpp = parseFloat(set.vmpp.toFixed(2));

	update_set_values(set.v,set_obj_str[0]);
	update_set_values(set.i,set_obj_str[1]);
	
	switch(state.mode)
	{
		case Mode_e.VCP:
			update_set_values(set.p,set_obj_str[2]);
			break;

		case Mode_e.VCR:
			update_set_values(set.r,set_obj_str[3]);
			break;

		case Mode_e.PVsim:
			update_set_values(set.vmpp,set_obj_str[4]);
			update_set_values(set.impp,set_obj_str[5]);
			break;

		default:
			break;
	}
	// updating cloned set values also with the new set values.
	Object.assign(user_in_set,set);
}

// V: function that will update SET values in control page 
// V: it will take 2 parameters 
//          V: val --> can be anything in the set struct
//          V: str --> let's us know what is the values passed 
function update_set_values(val, str)
{
  val = Number(val);
  if (str !== null)
  {
    switch (str) {
      case "ua":
        // V: update set.v value
        document.getElementById("uRange").value = val;
        document.querySelector('input[type="number"].uaInput').value = val;
        break;

      case "ia":
        // V: update set.i value
        document.getElementById("iRange").value = val;
        document.querySelector('input[type="number"].iaInput').value = val;
        break;

      case "pa":
        // V: update set.p value
        document.getElementById("pRange").value = val;
        document.querySelector('input[type="number"].paInput').value = val;
        break;

      case "ra":
        // V: update set.r value
        document.getElementById("rRange").value = val;
        document.querySelector('input[type="number"].raInput').value = val;
        break;

      case "umpp":
        // V: update set.vmpp value
        document.getElementById("umppRange").value = val;
        document.querySelector('input[type="number"].umppInput').value = val;
        break;

      case "impp":
        // V: update set.impp value
        document.getElementById("imppRange").value = val;
        document.querySelector('input[type="number"].imppInput').value = val;
        break;

      default:
        break;
    }
  }
}

// V: this function is used to set the limits of Bars & text boxes in the control page
function update_dev_params(recv_devparam)
{
	if(recv_devparam)
	{
    // V: Updating the Received Device parameters into local variables
		DevParams.umax = recv_devparam[0];
		DevParams.imax = recv_devparam[1];
		DevParams.pmax = recv_devparam[2];
		DevParams.rmax = recv_devparam[3];
		DevParams.rmin = recv_devparam[4];

    // V: After decimal points only showing 2 digits
		DevParams.umax = parseFloat(DevParams.umax.toFixed(2));
		DevParams.imax = parseFloat(DevParams.imax.toFixed(2));
		DevParams.pmax = parseFloat(DevParams.pmax.toFixed(2));
		DevParams.rmax = parseFloat(DevParams.rmax.toFixed(2));
		DevParams.rmin = parseFloat(DevParams.rmin.toFixed(2));

    // V: Below lines are for updating limits in control page.
    // V: update voltage lim value
    document.getElementById("uRange").max = DevParams.umax;
    document.querySelector('input[type="number"].uaInput').max = DevParams.umax;

    // V: update current lim value
    document.getElementById("iRange").max = DevParams.imax;
    document.querySelector('input[type="number"].iaInput').max = DevParams.imax;

    // V: update Power lim value
    document.getElementById("pRange").max = DevParams.pmax;
    document.querySelector('input[type="number"].paInput').max = DevParams.pmax;

    // V: update resistance higher lim value
    document.getElementById("rRange").max = DevParams.rmax;
    document.querySelector('input[type="number"].raInput').max = DevParams.rmax;

    // V: Update resistance lower lim value
    document.getElementById("rRange").min = DevParams.rmin;
    document.querySelector('input[type="number"].raInput').min = DevParams.rmin;

    // V: update Vmpp lim value
    document.getElementById("umppRange").max = DevParams.umax;
    document.querySelector('input[type="number"].umppInput').max = DevParams.umax;

    // V: update Impp lim value
    document.getElementById("imppRange").max = DevParams.imax;
    document.querySelector('input[type="number"].imppInput').max = DevParams.imax;

    // V: Update local variables based on devparam's
    Vslope_max = (DevParams.umax * 100);
    Islope_max = (DevParams.imax * 100);
    Ovp_max    = (DevParams.umax * 1.2);
    Uvp_max    = DevParams.umax;
    Ocp_max    = DevParams.imax;


    // V: Below lines are for updating limits in Configuartion page
    // V: Update V Limit max value
    document.getElementById("VlimRange").max = DevParams.umax;
    document.querySelector('input[type="number"].VlimInput').max =DevParams.umax;

    // V: Update C limit max limits
    document.getElementById("ClimRange").max = DevParams.imax;
    document.querySelector('input[type="number"].ClimInput').max = DevParams.imax;

    // V: Update Vslope max limits
    document.getElementById("VSlopeRange").max = Vslope_max;
    document.querySelector('input[type="number"].VSlopeInput').max = Vslope_max;

    // V: Update Islope max limits
    document.getElementById("ISlopeRange").max = Islope_max;
    document.querySelector('input[type="number"].ISlopeInput').max = Islope_max;

    // V: Update TEnable max limits
    document.getElementById("TEnableRange").max = TEnable_max;
    document.querySelector('input[type="number"].TEnableInput').max = TEnable_max;

    // V: Update OutputOnDelayRange max limits
    document.getElementById("OutputOnDelayRange").max = OutputOnDelay_max;
    document.querySelector('input[type="number"].OutputOnDelayInput').max = OutputOnDelay_max;

    // V: Update DataLogging max limits
    document.getElementById("DataLoggingRange").max = DataLogging_max;
    document.querySelector('input[type="number"].DataLoggingInput').max = DataLogging_max;


    // V: Below lines are for updating limits in Protection page
    // V: Update Ovp max limits
    document.getElementById("OvpRange").max = Ovp_max;
    document.querySelector('input[type="number"].OvpInput').max = Ovp_max;

    // V: Update Uvp max limits
    document.getElementById("UvpRange").max = Uvp_max;
    document.querySelector('input[type="number"].UvpInput').max = Uvp_max;

    // V: Update Ocp max limits
    document.getElementById("OcpRange").max = Ocp_max;
    document.querySelector('input[type="number"].OcpInput').max = Ocp_max;

    // V: Update UvpT max limits
    document.getElementById("UvpTRange").max = UvpT_max;
    document.querySelector('input[type="number"].UvpTInput').max = UvpT_max;

    // V: Update OcpT max limits
    document.getElementById("OcpTRange").max = OcpT_max;
    document.querySelector('input[type="number"].OcpTInput').max = OcpT_max;

    // V: Update FoldbackTm max limits
    document.getElementById("FoldbakTmRange").max = FoldbackTm_max;
    document.querySelector('input[type="number"].FoldbakTmInput').max = FoldbackTm_max;
	}
}

// V: function that will update the Status struct value into Web-Interface.
function Update_status(recv_status) 
{
  // V: Copying the received values from Websocket into local variables
  state.power = recv_status[0];
  state.mode = recv_status[1];
  state.GotoLocal = recv_status[2];
  state.intLockMode = recv_status[3];
  state.limMode = recv_status[4];
  state.source = recv_status[5];
  state.interlock = recv_status[6];
  state.LocalLockout = recv_status[7];
  state.fault = recv_status[8];

  // V: cloning the status struct values into user statu structre
  Object.assign(user_in_status, state);
  Mode_updation_UX_UI(state.mode);

  if (identify_page != Page_e.NA_e) 
  {
    // V: Switch case for updating the power state of the device
    switch (state.power) 
    {
      case Power_e.ON:
        document.querySelector(".statusText").textContent = "Run";
        document.querySelector("#curr_stat").textContent = "Run";
        document.getElementById("Run_Button").textContent = "Standby";
        break;
      case Power_e.OFF:
        document.querySelector(".statusText").textContent = "Standby";
        document.querySelector("#curr_stat").textContent = "Standby";
        document.getElementById("Run_Button").textContent = "Run";
        break;
      case Power_e.ON_PEND:
        document.querySelector(".statusText").textContent = "RUN_PEND";
        document.querySelector("#curr_stat").textContent = "RUN_PEND";
        document.getElementById("Run_Button").textContent = "RUN_PEND";
        break;
      case Power_e.OFF_PEND:
        document.querySelector(".statusText").textContent = "Standby_PEND";
        document.querySelector("#curr_stat").textContent = "Standby_PEND";
        document.getElementById("Run_Button").textContent = "Standby_PEND";
        break;
      default:
        break;
    }

    if ((state.power == Power_e.ON || state.power == Power_e.OFF) && state.fault != Fault_e.NA) {
      // V: Switch case for updating the Fault state of the device
      switch (state.fault) {
        case Fault_e.Ovp:
          document.querySelector(".statusText").textContent = "Ovp";
          document.querySelector("#curr_stat").textContent = "Ovp";
          break;
        case Fault_e.Ocp:
          document.querySelector(".statusText").textContent = "Ocp";
          document.querySelector("#curr_stat").textContent = "Ocp";
          break;
        case Fault_e.Uvp:
          document.querySelector(".statusText").textContent = "Uvp";
          document.querySelector("#curr_stat").textContent = "Uvp";
          break;
        case Fault_e.Otp:
          document.querySelector(".statusText").textContent = "OverTemp";
          document.querySelector("#curr_stat").textContent = "OverTemp";
          break;
        default:
          break;
      }
    }

    switch (state.mode) {
      // V: Switch case for updating the Mode of the device
      case Mode_e.VC:
        document.querySelector(".modeText").textContent = "UI";
        document.querySelector(".modeInput").value = "UI";
        break;
      case Mode_e.VCP:
        document.querySelector(".modeText").textContent = "UIP";
        document.querySelector(".modeInput").value = "UIP";
        break;
      case Mode_e.VCR:
        document.querySelector(".modeText").textContent = "UIR";
        document.querySelector(".modeInput").value = "UIR";
        break;
      case Mode_e.PVsim:
        document.querySelector(".modeText").textContent = "PVsim";
        document.querySelector(".modeInput").value = "PVsim";
        break;
      case Mode_e.User:
      case Mode_e.Script:
        showError("User & Script modes are in Progress Currently.");
        break;
      default:
        break;
    }

    switch (state.limMode) {
      // V: Switch case for updating the Limmode(CV/CC/CP) state of the device
      case LimMode_e.CV:
        document.querySelector(".limitText").textContent = "U";
        document.querySelector("#curr_lim").textContent = "U";
        break;
      case LimMode_e.CC:
        document.querySelector(".limitText").textContent = "I";
        document.querySelector("#curr_lim").textContent = "I";
        break;
      case LimMode_e.CP:
        document.querySelector(".limitText").textContent = "P";
        document.querySelector("#curr_lim").textContent = "P";
        break;
      default:
        break;
    }
  }
}

// V: function that will update the Preset sliders based on the mode.
function Mode_updation_UX_UI(which_mode) 
{
  document.getElementById("uRow").style.display = "flex";
  document.getElementById("iRow").style.display = "flex";
  document.getElementById("pRow").style.display = "none";
  document.getElementById("rRow").style.display = "none";
  document.getElementById("umppRow").style.display = "none";
  document.getElementById("imppRow").style.display = "none";

  // V: Only showing the sliders of the Respected mode
  if (which_mode === Mode_e.VCP) {
    document.getElementById("pRow").style.display = "flex";
  } else if (which_mode === Mode_e.VCR) {
    document.getElementById("rRow").style.display = "flex";
  } else if (which_mode === Mode_e.PVsim) {
    document.getElementById("umppRow").style.display = "flex";
    document.getElementById("imppRow").style.display = "flex";
  }
}

// V: function that will update the source of the device.
function update_source(src) 
{
  switch (src) {
    case Local_Source_e.Local:
      document.querySelector("#curr_ctrl").textContent = "Local";
      document.getElementById("Local_Button").textContent = "WLAN";
      document.querySelector(".localText").textContent = "Local";
      break;
    case Local_Source_e.WLAN:
      document.querySelector("#curr_ctrl").textContent = "WLAN";
      document.getElementById("Local_Button").textContent = "Local";
      document.querySelector(".localText").textContent = "WLAN";
      break;
    default:
      break;
  }
}

// V: function that will update All Configuration page values for first time when DOM is loaded
function update_all_configuration_page_values_first_time()
{
  update_configuration_page_values("Vlim", set.vLim);
  update_configuration_page_values("Clim", set.iLim);
  update_configuration_page_values("VSlope", set.vSlope);
  update_configuration_page_values("ISlope", set.vSlope);
  update_configuration_page_values("TEnable", configuration_params.TEnable);
  update_configuration_page_values("RemLastSettings", configuration_params.RemLastsettings);
  update_configuration_page_values("OutputOnDelay", configuration_params.Tdelay);
  update_configuration_page_values("DataLogging", configuration_params.DataLogging);
}

// V: function that will update configuration page values
function update_configuration_page_values(str,val)
{
  if(!str)
  {
    return;
  }
  val = Number(val);
  switch(str)
  {
    case "Vlim":
      document.getElementById("VlimRange").value = val;
      document.querySelector('input[type="number"].VlimInput').value = val;
    break;

    case "Clim":
      document.getElementById("ClimRange").value = val;
      document.querySelector('input[type="number"].ClimInput').value = val;
    break;

    case "VSlope":
      document.getElementById("VSlopeRange").value = val;
      if(val <= 0)
      {
        document.querySelector('input[type="number"].VSlopeInput').value = "";
        document.querySelector('input[type="number"].VSlopeInput').placeholder = "Off";
      }
      else{
        document.querySelector('input[type="number"].VSlopeInput').placeholder = "";
        document.querySelector('input[type="number"].VSlopeInput').value = val;
      }
      
    break;

    case "ISlope":
      document.getElementById("ISlopeRange").value = val;
      if(val <= 0)
      {
        document.querySelector('input[type="number"].ISlopeInput').value = "";
        document.querySelector('input[type="number"].ISlopeInput').placeholder = "Off";
      }
      else
      {
        document.querySelector('input[type="number"].ISlopeInput').placeholder = "";
        document.querySelector('input[type="number"].ISlopeInput').value = val;
      }
    break;

    case "TEnable":
      document.getElementById("TEnableRange").value = val;
      if(val <= 0)
      {
        document.querySelector('input[type="number"].TEnableInput').value = "";
        document.querySelector('input[type="number"].TEnableInput').placeholder = "Infinite";
      }
      else
      {
        document.querySelector('input[type="number"].TEnableInput').placeholder = "";
        document.querySelector('input[type="number"].TEnableInput').value = val;
      }
    break;

    case "RemLastSettings":
      if(val)
      {
        document.getElementById("RemLastSettingsRow").value = "On";
      }
      else
      {
        document.getElementById("RemLastSettingsRow").value = "Off";
      }
    break;

    case "OutputOnDelay":
      document.getElementById("OutputOnDelayRange").value = val;
      if(val <= 0)
      {
        document.querySelector('input[type="number"].OutputOnDelayInput').value = "";
        document.querySelector('input[type="number"].OutputOnDelayInput').placeholder = "Off";
      }
      else
      {
        document.querySelector('input[type="number"].OutputOnDelayInput').placeholder = "";
        document.querySelector('input[type="number"].OutputOnDelayInput').value = val;
      }
    break;

    case "DataLogging":
      document.getElementById("DataLoggingRange").value = val;
      if(val <= 0)
      {
        document.querySelector('input[type="number"].DataLoggingInput').value = "";
        document.querySelector('input[type="number"].DataLoggingInput').placeholder = "Off";
      }
      else
      {
        document.querySelector('input[type="number"].DataLoggingInput').value = val;
        document.querySelector('input[type="number"].DataLoggingInput').placeholder = "";
      }
    break;

    default:
    break;
  }
}

// V: function that will update all Protection page values for the first time when DOM is loaded.
function update_all_Protection_page_values_first_time(InitParams_data)
{
  if(InitParams_data)
  {
    arr_To_Obj(configuration_params,InitParams_data);
  }
  update_Protection_page_values("Ovp",set.ovp);
  update_Protection_page_values("Uvp",set.uvp);
  update_Protection_page_values("Ocp",set.ocp);
  update_Protection_page_values("UvpT",set.uvpT);
  update_Protection_page_values("OcpT",set.ocpT);
  // update_Protection_page_values("Foldback",set.ovp); // V: currently no command implemented.
}

// V: function that will update the Protection page values
function update_Protection_page_values(protection_str,protection_val)
{
  if(!protection_str)
  {
    return;
  }

  switch(protection_str)
  {
    case "Ovp":
      document.getElementById("OvpRange").value = protection_val;
      document.querySelector('input[type="number"].OvpInput').value = protection_val;
      break;

    case "Uvp":
      document.getElementById("UvpRange").value = protection_val;
      if(protection_val == -1)
      {
        document.querySelector('input[type="number"].UvpInput').placeholder = "Off";
        document.querySelector('input[type="number"].UvpInput').value = "";
      }
      else
      {
        document.querySelector('input[type="number"].UvpInput').value = protection_val;
        document.querySelector('input[type="number"].UvpInput').placeholder = "";
      }
      break;

    case "Ocp":
      document.getElementById("OcpRange").value = protection_val;
      if(protection_val == -1)
      {
        document.querySelector('input[type="number"].OcpInput').value = "";
        document.querySelector('input[type="number"].OcpInput').placeholder = "Off";
      }
      else
      {
        document.querySelector('input[type="number"].OcpInput').value = protection_val;
        document.querySelector('input[type="number"].OcpInput').placeholder = "";
      }
      break;

    case "UvpT":
      document.getElementById("UvpTRange").value = protection_val;
      document.querySelector('input[type="number"].UvpTInput').value = protection_val;
      break;

    case "OcpT":
      document.getElementById("OcpTRange").value = protection_val;
      document.querySelector('input[type="number"].OcpTInput').value = protection_val;
      break;

    case "Foldback":
      switch(protection_val)
      {
        case FoldBack_e.Fold_OFF:
          document.getElementById("foldbackRow").value = "off";
          break;
        
        case FoldBack_e.Fold_CV:
          document.getElementById("foldbackRow").value = "cv";
          break;

        case FoldBack_e.Fold_CC:
          document.getElementById("foldbackRow").value = "cc";
          break;
        
        case FoldBack_e.Fold_CP:
          document.getElementById("foldbackRow").value = "cp";
          break;

        default:
          break;
      }
      break;

    case "FoldbakTm":
      document.getElementById("FoldbakTmRange").value = protection_val;
      document.querySelector('input[type="number"].FoldbakTmInput').value = protection_val;
      break;

    default:
      break;

  }
}

// --------------- User input functions will be called directly inline from HTML --------------
// --------------- Aslo Event Listener Functions ----------------------------------------------

// V: function that will be called when mode is changed
function mode_input(mode) 
{
  if (!mode) 
  {
    return;
  }

  if (state.power === Power_e.ON) 
  {
    showError("Changing mode is Restricted,\n when device is running.");
    return;
  }
  switch (mode) 
  {
    case "UI":
      user_in_status.mode = Mode_e.VC;
      break;
    case "UIP":
      user_in_status.mode = Mode_e.VCP;
      break;
    case "UIR":
      user_in_status.mode = Mode_e.VCR;
      break;
    case "PVsim":
      user_in_status.mode = Mode_e.PVsim;
    default:
      break;
  }
  Send_Status_Struct();
  Mode_updation_UX_UI(user_in_status.mode);
}

// V: Function that will be called when Run button is clicked.
function Run_btn_click()
{
  var Runbtn = document.getElementById("Run_Button");
  switch (Runbtn.textContent.trim())
  {
    case "Run":
      Runbtn.textContent = "Standby";
      user_in_status.power = Power_e.ON;
      document.querySelector("#curr_stat").textContent = "Run";
      break;

    case "Standby":
      Runbtn.textContent = "Run";
      user_in_status.power = Power_e.OFF;
      document.querySelector("#curr_stat").textContent = "Standby";
      break;

    default:
      break;
  }
  Send_Status_Struct();
}

// V: Function that will be called when local button is
function Local_btn_click()
{
  var Localbtn = document.getElementById("Local_Button");
  switch (Localbtn.textContent.trim()) 
  {
    case "WLAN":
      Localbtn.textContent = "Local";
      user_in_local_src = Local_Source_e.WLAN;
      document.querySelector("#curr_ctrl").textContent = "WLAN";
      document.querySelector(".localText").textContent = "WLAN";
      break;

    case "Local":
      Localbtn.textContent = "WLAN";
      user_in_local_src = Local_Source_e.Local;
      document.querySelector("#curr_ctrl").textContent = "Local";
      document.querySelector(".localText").textContent = "Local";
      break;

    default:
      break;
  }
  
  local_src = user_in_local_src;
  ws.send(
    concatBytes(
      Uint8Array.of(Messages_e.SOURCE_LOCAL),
      Uint8Array.of(user_in_local_src)
    )
  );
}
  
// V: Function that will be called when Preset value input is entered.
function setval_input(str, val)
{
  // V: Checking if passed parameters are valid or not.
	if(!str)
	{
    Error_Message("Please provide a valid input.");
    return;
  }

  var sendarr = null;
  const Float_Tolerance = 1e-10;
  val = Number(val);
  switch (str) 
  {
    case 'ua':
      sendarr = Uint8Array.of(Messages_e.SET_V);
      if(val > (set.vLim + Float_Tolerance)) {
        val = set.vLim;
      }
      else if(val <= 0.0)
      {
        val = 0;
      }
      set.v = val;
      break;

    case 'ia':
      sendarr = Uint8Array.of(Messages_e.SET_I);
      if(val > (set.iLim + Float_Tolerance)) {
        val = set.iLim;
      }
      else if(val <= 0.0)
      {
        val = 0;
      }
      set.i = val;
      break;

    case 'pa':
      sendarr = Uint8Array.of(Messages_e.SET_P);
      if (val >= DevParams.pmax) {
        val = DevParams.pmax;
      }
      else if(val <= 0)
      {
        val = 0;
      }
      set.p = val;
      break;

    case 'ra':
      sendarr = Uint8Array.of(Messages_e.SET_R);
      if (val >= DevParams.rmax) {
        val = DevParams.rmax;
      }
      else if(val <= DevParams.rmin)
      {
        val = DevParams.rmin;
      }
      set.r = val;
      break;

    case 'umpp':
      // V: Umpp min = (10 * set voltage)/19;
      // V: Umpp max = 0.95 * set voltage;
      sendarr = Uint8Array.of(Messages_e.SET_VMPP);
      if(val >= (0.95 * set.v))
        val = 0.95 * set.v;
      else if(val <= (10 * set.v)/19)
        val = (10 * set.v)/19;
      val = val.toFixed(2);
      set.vmpp = val;
      break;

    case 'impp':
      // V: Impp min = (10 * set current)/19;
      // V: Impp max = 0.95 * set current;
      if(val >= (0.95 * set.i))
        val = 0.95 * set.i;
      else if(val <= (10 * set.i)/19)
        val = (10 * set.i)/19;
      val = val.toFixed(2);
      sendarr = Uint8Array.of(Messages_e.SET_IMPP);
      set.impp = val;
      break;

    default:
      break;
  }
  update_set_values(val, str);
  ws.send(concatBuffers(sendarr, Float32Array.of(val)));
}

// V: Set value Drag Event listener that will be called when using slider.
function setval_drag(str, val) 
{
  // V: Value can be 0 Sometimes that'why chceking only recvived string
  if (!str)
  {
    return;
  }

  switch (str) 
  {
    case "ua":
      document.querySelector('input[type="number"].uaInput').value = val;
      break;
    case "ia":
      document.querySelector('input[type="number"].iaInput').value = val;
      break;
    case "pa":
      document.querySelector('input[type="number"].paInput').value = val;
      break;
    case "ra":
      document.querySelector('input[type="number"].raInput').value = val;
      break;
    case "umpp":
      document.querySelector('input[type="number"].umppInput').value = val;
      break;
    case "impp":
      document.querySelector('input[type="number"].imppInput').value = val;
    default:
      break;
  }
}

// --------------->   Configuration Pages Input Functions   <----------------
// V: function that will be called when configuration parameters are changing.
function config_page_input(config_str,config_val)
{
  if(!config_str)
  {
    return;
  }
  var send_cmd_config = null;
  var integer_flag = false;  // V: If True send Float data else send integer data.
  if(config_str != "RemLastSettings")
  {
    config_val = Number(config_val);
  }
  switch(config_str)
  {
    case "Vlim":
      if(config_val >= DevParams.umax)
        config_val = DevParams.umax;
      else if(config_val <= 0.0)
        config_val = 0.0;
      send_cmd_config = Uint8Array.of(Messages_e.SET_VLIM);
      set.vLim = config_val;
      if(set.v > set.vLim)
        setval_input("ua",set.v);
    break;

    case "Clim":
      if(config_val >= DevParams.imax)
        config_val = DevParams.imax;
      else if(config_val <= 0.00)
        config_val = 0.00;
      send_cmd_config = Uint8Array.of(Messages_e.SET_ILIM);
      set.iLim = config_val;
      if(set.i > set.iLim)
        setval_input("ia",set.iLim);
    break;

    case "VSlope":
      if(config_val >= Vslope_max)
        config_val = Vslope_max;
      else if(config_val <= 0)  
        config_val = 0;
      send_cmd_config = Uint8Array.of(Messages_e.SET_V_SL);
      set.vSlope = config_val;
    break;

    case "ISlope":
      if(config_val >= Islope_max)
        config_val = Islope_max;
      else if(config_val <= 0)    
        config_val = 0;
      send_cmd_config = Uint8Array.of(Messages_e.SET_I_SL);
      set.iSlope = config_val;
    break;

    case "TEnable":
      if(config_val >= TEnable_max)
        config_val = TEnable_max;
      else if(config_val <= 0)    
        config_val = 0;
      send_cmd_config = Uint8Array.of(Messages_e.TENABLE);
      configuration_params.TEnable = config_val;
      integer_flag = true;
    break;

    case "RemLastSettings":
      send_cmd_config = Uint8Array.of(Messages_e.REMEMBER);
      if(config_val === "On")
      {
        configuration_params.RemLastsettings = Last_Settings_e.ON;
        config_val = Last_Settings_e.ON;
      }
      else
      {
        configuration_params.RemLastsettings = Last_Settings_e.OFF;
        config_val = Last_Settings_e.OFF;
      }
      integer_flag = true;
    break;

    case "OutputOnDelay":
      if(config_val >= OutputOnDelay_max)
        config_val = OutputOnDelay_max;
      else if(config_val <= 0)    
        config_val = 0;
      send_cmd_config = Uint8Array.of(Messages_e.TDELAY);
      configuration_params.Tdelay = config_val;
      integer_flag = true;
    break;

    case "DataLogging":
      if( config_val >= DataLogging_max)
        config_val = DataLogging_max;
      else if(config_val <= 0)    
        config_val = 0;
      send_cmd_config = Uint8Array.of(Messages_e.LOGGER);
      configuration_params.DataLogging = config_val;
      integer_flag = true;
    break;

    default:
    break;
  }

  update_configuration_page_values(config_str,config_val);
  if(integer_flag)
  {
    ws.send(concatBuffers(send_cmd_config, Uint8Array.of(config_val)));
  }
  else
  {
    ws.send(concatBuffers(send_cmd_config, Float32Array.of(config_val)));
  }
  
}

// V: function that will be called when config page sliders are dragged.
function config_page_slider_drag(config_slider_str,config_slider_val)
{
  if(!config_slider_str)
  {
    return;
  }

  switch(config_slider_str)
  {
    case "Vlim":
      document.querySelector('input[type="number"].VlimInput').value = config_slider_val;
      break;
    
    case "Clim":
      document.querySelector('input[type="number"].ClimInput').value = config_slider_val;
      break;
  
    case "VSlope":
      document.querySelector('input[type="number"].VSlopeInput').value = config_slider_val;
    break;

    case "ISlope":
      document.querySelector('input[type="number"].ISlopeInput').value = config_slider_val;
    break;

    case "TEnable":
      document.querySelector('input[type="number"].TEnableInput').value = config_slider_val;
    break;

    case "OutputOnDelay":
      document.querySelector('input[type="number"].OutputOnDelayInput').value = config_slider_val;
    break;

    case "DataLogging":
      document.querySelector('input[type="number"].DataLoggingInput').value = config_slider_val;
    break;

    default:
    break;
  }
}

// ----------------->    Protection page input functions    <--------------
// V: function that will be called when protection page value input has come
function Protection_page_input(protection_str,protection_val)
{
  if(!protection_str)
  {
    return;
  }
  var send_cmd_protection = null;
  int_flag = false;
  var new_protection_val = 0;
  if(protection_str != "Foldback")
    protection_val = Number(protection_val);

  switch(protection_str)
  {
    case "Ovp":
      if(protection_val >= Ovp_max)
        protection_val = Ovp_max;
      else if(protection_val <= 0.00)
        protection_val = 0.00;
      send_cmd_protection = Uint8Array.of(Messages_e.SET_OVP);
      set.ovp = protection_val;
      break;

    case "Uvp":
      if(protection_val >= Uvp_max)
        protection_val = Uvp_max;
      else if(protection_val <= 0.0)
        protection_val = -1;
      send_cmd_protection = Uint8Array.of(Messages_e.SET_UVP);
      set.uvp = protection_val;
      break;

    case "Ocp":
      if(protection_val >= Ocp_max)
        protection_val = Ocp_max;
      else if(protection_val <= 0.00)
        protection_val = -1;
      send_cmd_protection = Uint8Array.of(Messages_e.SET_OCP);
      set.ocp = protection_val;
      break;

    case "UvpT":
      if(protection_val >= UvpT_max)
        protection_val = UvpT_max
      else if(protection_val <= 100)
        protection_val = 100;
      send_cmd_protection = Uint8Array.of(Messages_e.SET_UVPT);
      set.uvpT = protection_val;
      break;

    case "OcpT":
      if(protection_val >= OcpT_max)
        protection_val = OcpT_max;
      else if(protection_val <= 1)
        protection_val = 1;
      send_cmd_protection = Uint8Array.of(Messages_e.SET_OCPT);
      set.ocpT = protection_val;
      break;

    case "Foldback":
      switch(protection_val)
      {
        case "off":
          new_protection_val = FoldBack_e.Fold_OFF;
          break;
        
        case "cv":
          new_protection_val = FoldBack_e.Fold_CV;
          break;

        case "cc":
          new_protection_val = FoldBack_e.Fold_CC;
          break;
        
        case "cp":
          new_protection_val = FoldBack_e.Fold_CP;
          break;

        default:
          break;
      }
      send_cmd_protection = Uint8Array.of(Messages_e.FOLDBACK);
      FoldBack.Foldback = new_protection_val;
      int_flag = true;
      break;

    case "FoldbakTm":
      if(protection_val >= FoldbackTm_max)
        protection_val = FoldbackTm_max;
      else if(protection_val <= FoldbackTm_min)
        protection_val = FoldbackTm_min;
      send_cmd_protection = Uint8Array.of(Messages_e.FOLDBACKTM);
      FoldBack.FoldbackTm = protection_val;
      break;

    default:
      break;
  }
  update_Protection_page_values(protection_str,protection_val);
  if(int_flag)
  {
    ws.send(concatBuffers(send_cmd_protection, Uint32Array.of(new_protection_val)));
  }
  else
  {
    ws.send(concatBuffers(send_cmd_protection, Float32Array.of(protection_val)));
  }
}

// V: function that will be called when protection page sliders are dragged
function protection_page_slider_drag(protection_slider_str,protection_slider_val)
{
  if(!protection_slider_str)
  {
    return;
  }

  switch(protection_slider_str)
  {
    case "Ovp":
      document.querySelector('input[type="number"].OvpInput').value = protection_slider_val;
      break;

    case "Uvp":
      document.querySelector('input[type="number"].UvpInput').value = protection_slider_val;
      break;

    case "Ocp":
      document.querySelector('input[type="number"].OcpInput').value = protection_slider_val;
      break;

    case "UvpT":
      document.querySelector('input[type="number"].UvpTInput').value = protection_slider_val;
      break;

    case "OcpT":
      document.querySelector('input[type="number"].OcpTInput').value = protection_slider_val;
      break;

    case "FoldbakTm":
      document.querySelector('input[type="number"].FoldbakTmInput').value = protection_slider_val;
      break;

    default:
      break;
  }
}

// --------------- Websocket Send function definitions --------------

//V: Funtion that will send Different commands about Websocket information.
function send_keepAlive(msg){
	if(is_onboard)
  {
    switch(msg)
    {
      case Websocket_com_e.DEAD:
        console.log("Sending Dead to server.");
        break;
      
      case Websocket_com_e.ALIVE:
        console.log("Pong to Server --> ");
        break;

      case Websocket_com_e.NO_DATA:
        console.log("Sending No data to server.");
        break;

      case Websocket_com_e.NO_DATA:
        console.log("Sending Websocket Closing request to server.");
        break;
    }
    ws.send(concatBuffers(Uint8Array.of(Messages_e.KP_ALIVE), Uint8Array.of(msg)));
	}
}

// V: Funtion that will send all the Status struct data to ESP
function Send_Status_Struct()
{
	// All the parameters of the status struct will be updated in their repective funtions in the user_in_status variable
		var statusArr = Uint8Array.of(Messages_e.NT_STATE);
		statusArr = concatBytes(statusArr, user_in_status.power);
		statusArr = concatBytes(statusArr, user_in_status.mode);
		statusArr = concatBytes(statusArr, user_in_status.GotoLocal);
		statusArr = concatBytes(statusArr, user_in_status.intLockMode);
		statusArr = concatBytes(statusArr, user_in_status.limMode);
		statusArr = concatBytes(statusArr, user_in_status.source);
		statusArr = concatBytes(statusArr, user_in_status.interlock);
		statusArr = concatBytes(statusArr, user_in_status.LocalLockout);
		ws.send(statusArr.buffer);
}


// --------------->   Additional Support function definitions   <--------------

// V: Funtion that is used to switch pages but not used any more
function switch_page(page) 
{
   document.getElementById("Index_Page").style.display = "none";
   document.getElementById("Control_Page").style.display = "none";
   document.getElementById("Display_Page").style.display = "none";
   document.getElementById("Protection_Page").style.display = "none";
   document.getElementById("Configuration_Page").style.display = "none";
   
  // V: Allowing only respective page to show at present time.
  switch (page) {
    case "Index":
      document.getElementById("Index_Page").style.display = "block";
      identify_page = Page_e.Index;
      break;

    case "Control":
      document.getElementById("Control_Page").style.display = "block";
      identify_page = Page_e.Control;
      break;

    case "Display":
      document.getElementById("Display_Page").style.display = "block";
      identify_page = Page_e.Display;
      break;
    
    case "Config":
      document.getElementById("Configuration_Page").style.display = "block";
      identify_page = Page_e.Configuration;
      break;
  
    case "Protection":
      document.getElementById("Protection_Page").style.display = "block";
      identify_page = Page_e.Protection;
      break;
      
    default :
      break;
  }
}

// V: The function that will convert the binary read msg from websocket into object values(structs)
function arr_To_Obj(obj, arr) {
	Object.keys(obj).map(function(key, index) {  // V: keys function will return all the key values(mems of object)
		obj[key] = arr[index];                   // V: map is an array function it acts like a loop
	});
}

// V: The function that will convert our object values into an array
function obj_To_Arr(obj, arr) {
	Object.keys(obj).map(function(key, index) {
		arr[index] = obj[key];
	});
}

// V: function that will convert float value into 4 bytes
function float_to_byte(value)
{
	var floatarr = new Float32Array(1);
	floatarr[1] = value;
	return new Uint8Array(floatarr.buffer);
}

function concatTypedArrays(a, b) { // a, b TypedArray of same type
    var c = new (a.constructor)(a.length + b.length);
    c.set(a, 0);
    c.set(b, a.length);
    return c;
}

function concatBuffers(a, b) {	// buffers of differtent TypedArrays
    return concatTypedArrays(
        new Uint8Array(a.buffer || a), 
        new Uint8Array(b.buffer || b)
    ).buffer;
}

function concatBytes(ui8a, byte) {
	return concatTypedArrays(ui8a, Uint8Array.of(byte));
}

// V: Function that will show error message if something happens.
function showError(message) {
  document.getElementById("errorModal").style.display = "block";
  document.getElementById("error_Message").innerText = message;

  // Auto-hide after 10 seconds
  setTimeout(hideError, 10000);
}

function hideError() {
  document.getElementById("errorModal").style.display = "none";
}

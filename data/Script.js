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
nopong_cnt = 0,                             // V: These 4 variables are used to keep the Webcocket communication Persistant.
keep_alive = 1,
keep_alive_count = 0,
got_response = 0;

var is_onboard = false;   // V: true  -> Onboard data flow will be continous
                          //    false -> Dev-kit data will not flow.


var user_in_set = Object.assign({}, set);           // V: Variable to store Preset values from user in Web-Interface
var user_in_status = Object.assign({}, state);      // V: Variable to store Status struct values from user in Web-Interface
var data_flow = 0;                                  // V: Variable to monitor the data flow b/w Server & Web-Interface
let set_arr = ["v", "i", "p", "r", "vmpp", "impp"]; // V: currently not in use.

// V: Hardware limits Structure some default limits are giving initially.
var DevParams = { umax: 700, imax: 70, pmax: 49000, rmax: 10, rmin: 0 };

// V: Preset values Structure
var set = {
    v: 0,
    i: 0,
    p: 0,
    r: 0,
    vmpp: 0,
    impp: 0,
    // V: down param's are not used currently.
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
    ANZ_MSG: 78,
  };

   
const Power_e = { ON: 0, OFF: 1, OFF_PEND: 2, ON_PEND: 3 };                 // V: Enum to describe the power statem device
const Mode_e = { VC: 0, VCP: 1, VCR: 2, PVsim: 3, User: 4, Script: 5 };     // V: Enum to describe the mode of device
const Local_Source_e = { Local: 0, WLAN: 1 };                               // V: Enum to describe the control of device(Ambiguties currently)
const Source_e = { Front: 0, AI: 1, Remote: 2 };                            // V: Ambiguties currently(Not in use currently.)
const Interlock_e = { lowAct: 0, highAct: 1 };                              // V: currenlt dont have the idea.
const LimMode_e = { CV: 0, CC: 1, CP: 2 };                                  // V: Enum to describe the CV/CC/CP of the device
const Fault_e = { NA: -1, Ovp: 0, Ocp: 1, Uvp: 2, Otp: 3 };                 // V: Enum to describe the Fault of the device
const set_obj_str = ["ua", "ia", "pa", "ra", "umpp", "impp"];               // V: Enum used for Differentiating b/w the set values
const Page_e = { NA_e: -1, Index: 0, Control: 1, Webconfig: 2, Display: 3, Protection : 4, Configuration : 5 } // V: Enum used for Differentiating b/w pages not used anymore.


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


// ------------------- Private functions Definitions ------------

// V: Starting function that will called in when the window is loaded.
window.onload = setup;

// V: Adding a event listener when webpage is closing so that 
window.addEventListener("beforeunload", function() {
    closeWebSocket();
});

// V: function to close the websocket 
function closeWebSocket() {
    if (ws) {
        console.log("Deleting Websocket");
        ws.close();
    }
}

// V: starting function to be called after page loading
function setup() 
{
  switch_page('Index');
    if(is_onboard)
    {
        ws_Connect();
        function_id = setInterval(ws_PingPong, 4000);
    }
    else
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
    if (is_onboard) {
      function_id2 = setInterval(Check_data_flow, 2200);
    }
  };

  ws.onclose = function () {
    console.log("Socket is getting closed.");
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
    //alert("Error in WLAN communication \nPlease check if WLAN is Connected.");
    ws.close();
  };
}

// V: function that will be called for every 4 seconds to send keep_alive to server.
function ws_PingPong() {
    if (got_response) 
    {
      send_keepAlive();
      got_response = false;
      nopong_cnt = 0;
    } else 
    {
      keep_alive = false;
      nopong_cnt++;
  
      if (nopong_cnt >= 2) 
      {
        clearInterval(function_id);
        alert("There is no connection to the WLAN access point \nPlease check if WLAN is enabled.");
        window.location.reload();
      } 
      else 
      {
        keep_alive = true;
        send_keepAlive();
        got_response = false;
      }
    }
  }

// V: function that will be called for every 2.2 seconds for monitoring data.
function Check_data_flow() 
{
    if (data_flow) {
      console.log("Communication is Persistent.");
      data_flow = false;
    } else {
      clearInterval(function_id2);
      console.log("Communication stopped from Server.");
      window.location.reload();
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
        break;

        // V: Device Programmable Current Limit
      case Messages_e.SET_ILIM:
        set.iLim = new Float32Array(e.data.slice(1, 5))[0];
        set.iLim = parseFloat(set.iLim.toFixed(2));
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
  
      case Messages_e.SET_V_SL:
      case Messages_e.SET_I_SL:
      case Messages_e.SET_OVP:
      case Messages_e.SET_UVP:
      case Messages_e.SET_UVPT:
      case Messages_e.SET_OCP:
      case Messages_e.SET_OCPT:
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
      case Messages_e.REMEMBER:
      case Messages_e.TDELAY:
      case Messages_e.TENABLE:
      case Messages_e.LOGGER:
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
			  // 1st -> monitor struct(4 float mem's) 					= 16 bytes
			  // 2nd -> preset struct(15 float mem's) 					= 60 bytes
			  // 3rd -> control struct(3 inner structs of 3 floats each)	= 36 bytes
			  // 4th -> status struct(8 uint's) 							= 8 bytes
        update_monitor_values(new Float32Array(e.data.slice(1, 17)));
        var s = new Float32Array(e.data.slice(153, 173));
        update_dev_params(s);
        call_update_set_values_func(new Float32Array(e.data.slice(17, 77)));
        Update_status(new Uint8Array(e.data.slice(113, 122)));
        break;

        // V: Keep alive case to keep Server & Client communication persistant. 
      case Messages_e.KP_ALIVE:
        if (t[1] == 1) {
          keep_alive = t[1];
          got_response = true;
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
    monitor.r = parseFloat(monitor.r.toFixed(3));

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
function call_update_set_values_func(recv_preset_val)
{
	if(recv_preset_val)
	{
		arr_To_Obj(set,recv_preset_val);
	}

	// V: conevrting all float values into values that will appear 2 numbers after decimal point
	set.i = parseFloat(set.i.toFixed(2));
	set.v = parseFloat(set.v.toFixed(2));
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
  if ((val !== null) && (str !== null)) 
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

    // V: update voltage lim value
    document.getElementById("uRange").max = DevParams.umax;
    document.querySelector('input[type="number"].uaInput').max = DevParams.umax;

    // V: update current lim value
    document.getElementById("iRange").max = DevParams.imax;
    document.querySelector('input[type="number"].iaInput').max = DevParams.imax;

    // V: update Power lim value
    document.getElementById("pRange").max = DevParams.pmax;
    document.querySelector('input[type="number"].paInput').max = DevParams.pmax;

    // V: update resistance lim value
    document.getElementById("rRange").max = DevParams.rmax;
    document.querySelector('input[type="number"].raInput').max = DevParams.rmax;

    // V: update Vmpp lim value
    document.getElementById("umppRange").max = DevParams.umax;
    document.querySelector('input[type="number"].umppInput').max = DevParams.umax;

    // V: update Impp lim value
    document.getElementById("imppRange").max = DevParams.imax;
    document.querySelector('input[type="number"].imppInput').max = DevParams.imax;
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
        document.querySelector(".modeText").textContent = "User";
        document.querySelector(".modeInput").value = "User";
        break;
      case Mode_e.Script:
        document.querySelector(".modeText").textContent = "Script";
        document.querySelector(".modeInput").value = "Script";
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
    alert("Sorry!! \nCan't change mode when Output is ON.");
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
  // V: Value can be 0 Sometimes that'why chceking only recvived string
	if(!str)
	{
    return;
  }

  var sendarr = null;
  switch (str) 
  {
    case 'ua':
      sendarr = Uint8Array.of(Messages_e.SET_V);
      if (val >= set.vLim) {
        val = set.vLim;
      }
      else if (val >= DevParams.umax) {
        val = DevParams.umax;
      }
      update_set_values(val, str);
      break;

    case 'ia':
      sendarr = Uint8Array.of(Messages_e.SET_I);
      if (val >= set.iLim) {
        val = set.iLim;
      }
      else if (val >= DevParams.imax) {
        val = DevParams.imax;
      }
      update_set_values(val, str);
      break;

    case 'pa':
      sendarr = Uint8Array.of(Messages_e.SET_P);
      if (val >= DevParams.pmax) {
        val = DevParams.pmax;
      }
      update_set_values(val, str);
      break;

    case 'ra':
      sendarr = Uint8Array.of(Messages_e.SET_R);
      if (val >= DevParams.rmax) {
        val = DevParams.rmax;
      }
      update_set_values(val, str);
      break;

    case 'umpp':
      sendarr = Uint8Array.of(Messages_e.SET_VMPP);
      update_set_values(val, str);
      break;

    case 'impp':
      sendarr = Uint8Array.of(Messages_e.SET_IMPP);
      update_set_values(val, str);
      break;

    default:
      break;
  }
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

// ----------------------->   Configuration Pages Input Functions   <---------   
// V: function that will be called when configuration parameters are changing.
function config_page_input(str,val)
{
  if(!str)
  {
    return;
  }

  switch(str)
  {
    case "Vlim":
    break;

    case "Clim":
    break;

    case "VSlope":
    break;

    case "ISlope":
    break;

    case "TEnable":
    break;

    case "RemLastSettings":
    break;

    case "OutputOnDelay":
    break;

    case "DataLogging":
    break;

    default:
    break;
  }
}

// V: function that will be called when config page sliders are dragged.
function config_page_slider_drag(str,val)
{
  if(!str)
  {
    return;
  }

  switch(str)
  {
    case "Vlim":
      document.querySelector('input[type="number"].VlimInput').value = val;
      break;
    
    case "Clim":
      document.querySelector('input[type="number"].ClimInput').value = val;
      break;
  
    case "VSlope":
      document.querySelector('input[type="number"].VSlopeInput').value = val;
    break;

    case "ISlope":
      document.querySelector('input[type="number"].ISlopeInput').value = val;
    break;

    case "TEnable":
      document.querySelector('input[type="number"].TEnableInput').value = val;
    break;

    case "OutputOnDelay":
      document.querySelector('input[type="number"].ClimInput').value = val;
    break;

    case "DataLogging":
      document.querySelector('input[type="number"].ClimInput').value = val;
    break;

    default:
    break;
  }
}


// ----------------->    Protection page input functions    <--------------
// V: function that will be called when protection parameters are changed.
function Protection_page_input(str,val)
{
  if(!str)
  {
    return;
  }

  switch(str)
  {
    case "Ovp":
    break;

    case "Uvp":
    break;

    case "Ocp":
    break;

    case "UvpT":
    break;

    case "OcpT":
    break;

    case "Foldback":
    break;

    default:
    break;
  }
}

// V: function that will be called when protection page sliders are dragged.
function protection_page_slider_drag(str,val)
{
  if(!str)
  {
    return;
  }

  switch(str)
  {
    case "Ovp":
      document.querySelector('input[type="number"].OvpInput').value = val;
    break;

    case "Uvp":
      document.querySelector('input[type="number"].UvpInput').value = val;
    break;

    case "Ocp":
      document.querySelector('input[type="number"].OcpInput').value = val;
    break;

    case "UvpT":
      document.querySelector('input[type="number"].UvpTInput').value = val;
    break;

    case "OcpT":
      document.querySelector('input[type="number"].OcpTInput').value = val;
    break;

    default:
    break;
  }
}


// --------------- Websocket Send function definitions --------------

//send keep_alive to check WS connection
function send_keepAlive(){
	if(is_onboard){
		 ws.send(concatBuffers(Uint8Array.of(Messages_e.KP_ALIVE), Uint8Array.of(keep_alive)));
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


// --------------- Additional Support function definitions --------------

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
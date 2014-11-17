
var Pexor;
var MyDisplay;



////////////// State class reflecting remote state and commnication:
function PexorState() {
	this.fDabcState="Running";
	this.fRunning = false;
	this.fFileOpen = false;
	this.fFileName = "run0.lmd";
	this.fFileInterval = 1000;
	this.fLogging = false;
	this.fLogData = null;

}

PexorState.prototype.DabcCommand = function(cmd, option, callback) {
	var xmlHttp = new XMLHttpRequest();
	var pre = "/PEXOR/";
	var suf = "/execute";
	var fullcom = pre + cmd + suf + "?" + option;
	console.log(fullcom);
	xmlHttp.open('GET', fullcom, true);
	xmlHttp.onreadystatechange = function() {

		if (xmlHttp.readyState == 4) {
			console.log("DabcCommand completed.");
			var reply = JSON.parse(xmlHttp.responseText);
			console.log("Reply= %s", reply);
			callback(true); // todo: evaluate return values of reply
		}
	}
	xmlHttp.send(null);
};


PexorState.prototype.DabcParameter = function(par, callback) {
	var xmlHttp = new XMLHttpRequest();
	var pre = "/PEXOR/";
	var suf = "/get.json";
	var fullcom = pre + par + suf; 
	console.log(fullcom);
	xmlHttp.open('GET', fullcom, true); 
	xmlHttp.onreadystatechange = function() {

		if (xmlHttp.readyState == 4) {
			//console.log("DabcParameter request completed.");
			var reply = JSON.parse(xmlHttp.responseText);
			 if (typeof reply != 'object') {
		         console.log("non-object in json response from server");
		         return;
		      }
			 
//			 var val= false;
//			 val=(reply['value']==="true") ? true : false;
			 //console.log("response=%s, Reply= %s, value=%s", xmlHttp.responseText, reply, val);
			 //console.log("name=%s, value= %s state=%s", reply['_name'], reply['value'], val);
			callback("true",reply['value']); 
		}
	}
	xmlHttp.send(null);
}; 



PexorState.prototype.UpdateRunstate = function(ok, state){
	//console.log("UpdateRunstate with ok=%s, value=%s", ok, state);
	if (ok=="true") {
		this.fRunning = (state==true);
	} else {
		console.log("UpdateRunstate failed.");
	}
}

PexorState.prototype.UpdateFilestate = function(ok, state){
	
	if (ok=="true") {
		this.fFileOpen = (state==true);
		} else {
		console.log("UpdateFilestate failed.");
	}
	//console.log("UpdateFilestate with ok=%s, value=%s, fileopen=%s, typeofFileopen=%s", ok, state, this.fFileOpen, typeof(this.fFileOpen));
}

PexorState.prototype.UpdateDABCstate = function(ok, state, refreshcall){
// DABC states as strings:	
//	"Halted"
//	"Ready"
//    "Running"
//    "Failure";
//    "Transition";	
	
	
	if (ok=="true") {
		this.fDabcState = state;			
	} else {
		console.log("UpdateDABCstate failed.");
	}
    console.log("UpdateDABCstate with ok=%s, value=%s, dabcstate=%s", ok, state, this.fDabcState);
    refreshcall();
}



PexorState.prototype.Update= function(callback){
	var pthis = this;
	//this.DabcParameter("PexDevice/PexorAcquisitionRunning", pthis.UpdateRunstate);
	//this.DabcParameter("PexReadout/PexorFileOn", pthis.UpdateFilestate);	
	this.DabcParameter("PexDevice/PexorAcquisitionRunning", function(res,val) { pthis.UpdateRunstate(res,val); });
	this.DabcParameter("PexReadout/PexorFileOn",function(res,val) { pthis.UpdateFilestate(res,val); })
	this.DabcParameter("App/State",function(res,val) { pthis.UpdateDABCstate(res,val, callback); })
	//this.fDabcState="Running";
	//callback(); // will be done when last parameter update response has been processed
}


/////////////// DISPLAY class to manage current view:
function PexorDisplay(state){
	this.fPexorState=state;
	this.fMonitoring=false;
	this.fTrending=false;
	this.fTrendingHistory=100;
	this.fUpdateTimer=null;
	this.fUpdateInterval=2000; // ms
	this.fGaugeEv = null;
	this.fGaugeDa = null;
	this.fTrendEv = null;
	this.fTrendDa = null;
	this.fLogDevice=null;
	this.fLogReadout=null;
}

// set up view elements of display:
PexorDisplay.prototype.BuildView = function(){
	
	
	
	this.fGaugeEv = new DABC.GaugeDrawElement();
	this.fTrendEv=new DABC.RateHistoryDrawElement();
	
	this.fGaugeDa = new DABC.GaugeDrawElement();
	this.fTrendDa=new DABC.RateHistoryDrawElement();
	
	this.fLogDevice= new DABC.LogDrawElement();
	this.fLogDevice.itemname = "/PEXOR/PexDevice/PexDevInfo";
	this.fLogDevice.EnableHistory(100);
	this.fLogDevice.CreateFrames($("#DeviceInfo"));
	
	this.fLogReadout= new DABC.LogDrawElement();
	this.fLogReadout.itemname = "/PEXOR/PexReadout/PexorInfo";
	this.fLogReadout.EnableHistory(100);
	this.fLogReadout.CreateFrames($("#ReadoutInfo"));
	
	this.SetRateGauges();
	
}


PexorDisplay.prototype.SetRateGauges = function(){
	
	this.fTrendEv.Clear();
	this.fTrendDa.Clear();
	this.fGaugeEv.itemname = "/PEXOR/PexReadout/PexorEvents";
	this.fGaugeEv.CreateFrames($("#EvRateDisplay"));
	this.fGaugeDa.itemname = "/PEXOR/PexReadout/PexorData";
	this.fGaugeDa.CreateFrames($("#DatRateDisplay"));
}

PexorDisplay.prototype.SetRateTrending = function(history){
	
	this.fTrendingHistory=history;
	this.fGaugeEv.Clear();
	this.fGaugeDa.Clear();
	this.fTrendEv.itemname="/PEXOR/PexReadout/PexorEvents";
	this.fTrendEv.EnableHistory(this.fTrendingHistory); 
	this.fTrendEv.CreateFrames($("#EvRateDisplay"));
	this.fTrendDa.itemname="/PEXOR/PexReadout/PexorData";
	this.fTrendDa.EnableHistory(this.fTrendingHistory);
	this.fTrendDa.CreateFrames($("#DatRateDisplay"));	
}



PexorDisplay.prototype.RefreshMonitor = function() {

	if (this.fTrending) {
		this.fTrendEv.force = true;
		
		this.fTrendEv.RegularCheck();
		this.fTrendEv.CheckResize(true);			
		this.fTrendDa.force = true;		
		this.fTrendDa.RegularCheck();
		this.fTrendDa.CheckResize(true);	
		
	} else {
		this.fGaugeEv.force = true;
		this.fGaugeEv.RegularCheck();
		this.fGaugeEv.CheckResize(true);	
		this.fGaugeDa.force = true;
		this.fGaugeDa.RegularCheck();
		this.fGaugeDa.CheckResize(true);

	}

	this.fLogDevice.force = true;
	this.fLogDevice.RegularCheck();
	this.fLogReadout.force = true;
	this.fLogReadout.RegularCheck();
	pthis = this;
	this.fPexorState.Update(function() {
		pthis.RefreshView()
	});
}





PexorDisplay.prototype.ChangeMonitoring = function(on){
	
    this.fMonitoring=on;
	if(on)
		{
		 this.SetStatusMessage("Starting monitoring timer with "+ this.fUpdateInterval + " ms.");
		 this.fUpdateTimer=window.setInterval(function(){MyDisplay.RefreshMonitor()}, this.fUpdateInterval);
		}
	else
		{
			window.clearInterval(this.fUpdateTimer);
			this.SetStatusMessage("Stopped monitoring timer.");
		}
	
}


PexorDisplay.prototype.SetTrending = function(on,history){
	this.fTrending=on;
	if(on)
		{
			console.log("SetTrending on");
			this.SetRateTrending(history); // todo: get interval from textbox
		}
	else
		{
			console.log("SetTrending off");
			this.SetRateGauges();
		}
	
}

PexorDisplay.prototype.RefreshView = function(){

	
	
	 if (this.fPexorState.fRunning) {
			$("#daq_container").addClass("styleGreen").removeClass("styleRed");
		} else {
			$("#daq_container").addClass("styleRed").removeClass("styleGreen");
		}
	 //console.log("RefreshView typeof fileopen=%s, value=%s globalvalue=%s", typeof(this.fPexorState.fFileOpen), 
	//		 this.fPexorState.fFileOpen, Pexor.fFileOpen);
	 
	 if (this.fPexorState.fFileOpen) {
		 	//console.log("RefreshView finds open file");
			$("#file_container").addClass("styleGreen").removeClass("styleRed");
			$("#buttonStartFile").prop('checked', true);
			 //$("label[for='buttonStartFile']").html("<span class=\"ui-button-text\"> Stop File </span>");
			$("label[for='buttonStartFile']").html("<span class=\"ui-button-icon-primary ui-icon ui-icon-closethick MyButtonStyle\"</span>");
			$("label[for='buttonStartFile']").attr("title", "Close output file");
		} else {
			//console.log("RefreshView finds close file");
			$("#file_container").addClass("styleRed").removeClass("styleGreen");
			$("#buttonStartFile").prop('checked', false);
			//$("label[for='buttonStartFile']").html("<span class=\"ui-button-text\"> Start File </span>");
			 $("label[for='buttonStartFile']").html("<span class=\"ui-button-icon-primary ui-icon ui-icon-disk MyButtonStyle\"</span>");
			 $("label[for='buttonStartFile']").attr("title", "Open lmd file for writing");
		}
	 
	 
	 if (this.fMonitoring) {
			$("#monitoring_container").addClass("styleGreen").removeClass("styleRed");
			 //$("label[for='Monitoring']").html("<span class=\"ui-button-text\">  Stop Monitoring </span>");
			 $("label[for='Monitoring']").html("<span class=\"ui-button-icon-primary ui-icon ui-icon-stop MyButtonStyle\"></span>");
			 $("label[for='Monitoring']").attr("title", "Stop frequent refresh");
		} else {
			$("#monitoring_container").addClass("styleRed").removeClass("styleGreen");
			//$("label[for='Monitoring']").html("<span class=\"ui-button-text\">  Start Monitoring </span>");
			$("label[for='Monitoring']").html("<span class=\"ui-button-icon-primary ui-icon ui-icon-play MyButtonStyle\"></span>");
			 $("label[for='Monitoring']").attr("title", "Activate frequent refresh");	
		}
	 
	 $("#Refreshtime").prop('disabled', this.fMonitoring);
	 
	 
	 if (this.fTrending) {
			 //$("label[for='Trending']").html("<span class=\"ui-button-text\">  Show Rate Gauges </span>");
			 $("label[for='Trending']").html("<span class=\"ui-button-icon-primary ui-icon ui-icon-circle-arrow-n MyButtonStyle\"></span>");//
			 //.addClass("MyButtonStyle");
			 $("label[for='Trending']").attr("title", "Show Rate Gauges");	
		} else {
			//$("label[for='Trending']").html("<span class=\"ui-button-text\">   Show Rate Trending </span>");
			$("label[for='Trending']").html("<span class=\"ui-button-icon-primary ui-icon ui-icon-image MyButtonStyle\"></span>");
			//.addClass("MyButtonStyle");
			$("label[for='Trending']").attr("title", "Show Rate Trending");		
		}
	 $("#Trendlength").prop('disabled', this.fTrending);
	 
	 console.log("RefreshView with dabc state = %s", this.fPexorState.fDabcState);
	 
	 if (this.fPexorState.fDabcState=="Running") { 
		//	console.log("RefreshView finds Running state");
			$("#dabc_container").addClass("styleGreen").removeClass("styleRed").removeClass("styleYellow").removeClass("styleBlue");
			 
		} 
	 else if (this.fPexorState.fDabcState=="Ready") {
		 //	console.log("RefreshView finds Ready state");
			$("#dabc_container").addClass("styleYellow").removeClass("styleRed").removeClass("styleGreen").removeClass("styleBlue");			 
		} 
	 else if ((this.fPexorState.fDabcState=="Failure") || (this.fPexorState.fDabcState=="Transition")) {
		 //console.log("RefreshView finds Failure state");
		 $("#dabc_container").addClass("styleBlue").removeClass("styleYellow").removeClass("styleRed").removeClass("styleGreen");			 
		} 	 
	 else {
		 //console.log("RefreshView finds other state");
			$("#dabc_container").addClass("styleRed").removeClass("styleGreen").removeClass("styleYellow").removeClass("styleBlue");
		}
	 
	 
};


PexorDisplay.prototype.SetStatusMessage= function(info) {
	var d = new Date();
	var txt = d.toLocaleString() + "  >" + info;
	document.getElementById("status_message").innerHTML = txt;
}





$(function() {

	Pexor = new PexorState();
	MyDisplay=new PexorDisplay(Pexor);
	MyDisplay.BuildView();
	MyDisplay.ChangeMonitoring(false);
	
	//////// first DABC generic commands
	// later this should be part of the framework...
	$("#buttonStartDabc").button({text: false, icons: { primary: "ui-icon-play MyButtonStyle"}}).click(
			function() {
				var requestmsg = "Really Re-Start DABC Application?";
				var response = confirm(requestmsg);
				if (!response)
					return;

				Pexor.DabcCommand("App/DoStart", "", function(
						result) {
					MyDisplay.SetStatusMessage(result ? "Start DABC command sent."
							: "Start DABC FAILED.");
					MyDisplay.RefreshMonitor();
				});
			});
	
	$("#buttonStopDabc").button({text: false, icons: { primary: "ui-icon-pause MyButtonStyle"}}).click(
			function() {
				var requestmsg = "Really Stop DABC Application?";
				var response = confirm(requestmsg);
				if (!response)
					return;

				Pexor.DabcCommand("App/DoStop", "", function(
						result) {
					MyDisplay.SetStatusMessage(result ? "Stop DABC command sent."
							: "Stop DABC  FAILED.");
					MyDisplay.RefreshMonitor();
				});
			});
	
	$("#buttonConfigureDabc").button({text: false, icons: { primary: "ui-icon-gear MyButtonStyle"}}).click(
			function() {
				var requestmsg = "Really Re-Configure DABC Application?";
				var response = confirm(requestmsg);
				if (!response)
					return;

				Pexor.DabcCommand("App/DoConfigure", "", function(
						result) {
					MyDisplay.SetStatusMessage(result ? "Configure DABC command sent."
							: "Configure DABC FAILED.");
					MyDisplay.RefreshMonitor();
				});
			});
	$("#buttonHaltDabc").button({text: false, icons: { primary: "ui-icon-stop MyButtonStyle"}}).click(
			function() {
				var requestmsg = "Really Halt (shutdown) DABC Application?";
				var response = confirm(requestmsg);
				if (!response)
					return;

				Pexor.DabcCommand("App/DoHalt", "", function(
						result) {
					MyDisplay.SetStatusMessage(result ? "Halt DABC command sent."
							: "Halt DABC FAILED.");
					MyDisplay.RefreshMonitor();
				});
			});
	
	
///////////////////////////// pexor specific:	
	
	$("#buttonStartAcquisition").button({text: false, icons: { primary: "ui-icon-play MyButtonStyle"}}).click(
			function() {
				var requestmsg = "Really Start Acquisition?";
				var response = confirm(requestmsg);
				if (!response)
					return;

				Pexor.DabcCommand("PexDevice/StartAcquisition", "", function(
						result) {
					MyDisplay.SetStatusMessage(result ? "Start Acquisition command sent."
							: "Start Acquisition FAILED.");
//					if (result)
//						Pexor.fRunning = true;
					MyDisplay.RefreshMonitor();
				});
			});

	$("#buttonStopAcquisition").button({text: false, icons: { primary: "ui-icon-pause MyButtonStyle"}}).click(
			function() {

				var requestmsg = "Really Stop Acquisition?";
				var response = confirm(requestmsg);
				if (!response)
					return;
				Pexor.DabcCommand("PexDevice/StopAcquisition", "", function(
						result) {
					MyDisplay.SetStatusMessage(result ? "Stop Acquisition command sent."
							: "Stop Acquisition FAILED.");
//					if (result)
//						Pexor.fRunning = false;

					MyDisplay.RefreshMonitor();
				});
			});
	$("#buttonInitAcquisition").button({text: false, icons: { primary: "ui-icon-wrench MyButtonStyle"}}).click(function() {
		var requestmsg = "Really Initialize Acquisition?";
		var response = confirm(requestmsg);
		if (!response)
			return;

		Pexor.DabcCommand("PexDevice/InitAcquisition","",function(
				result) {
			MyDisplay.SetStatusMessage(result ? "Init Acquisition command sent."
					: "Init Acquisition FAILED.");
			MyDisplay.RefreshMonitor();
			
		});

	});

	$("#buttonStartFile").button({text: false, icons: { primary: "ui-icon-disk MyButtonStyle"}})
			.click(
					function() {						
						var checked= $(this).is(':checked');
						
						if(!checked)
							{
							var requestmsg = "Really Stop writing output file "
								+ Pexor.fFileName + " ?";
						var response = confirm(requestmsg);
						if (!response)
							{
								$(this).prop('checked', true);
								return;
							}
						Pexor.DabcCommand("PexReadout/StopFile","",function(
								result) {
							MyDisplay.SetStatusMessage(result ? "Stop File command sent."
									: "Stop File FAILED.");
//							if (result)
//								Pexor.fFileOpen = false;
							// todo: evaluate real state of file open/close
							MyDisplay.RefreshMonitor();
						});
							
							
							}
						else
							{							
							var datafilename=document.getElementById("Filename").value;
							var datafilelimit=document.getElementById("Filesize").value;
							
						var requestmsg = "Really Start writing output file "
						+ datafilename + ", maxsize=" + datafilelimit +" ?";
					var response = confirm(requestmsg);
					if (!response)
						{
							$(this).prop('checked', false);						
							return;
						}
					
						var options = "FileName=" + datafilename
						 + "&maxsize=" + datafilelimit;

						Pexor.DabcCommand("PexReadout/StartFile", options,function(
								result) {
							MyDisplay.SetStatusMessage(result ? "Start File command sent with options "+options
									: "Start File FAILED.");
							if (result)
								{
								Pexor.fFileName = datafilename;
								}
							MyDisplay.RefreshMonitor();
							
							
						});
							}
						
						

					});



	$("#Monitoring").button({text: false, icons: { primary: "ui-icon-play MyButtonStyle"}}).click(function() {		
		//MyDisplay.fUpdateInterval= Number($("#Refreshtime").value); // does not work?
		MyDisplay.fUpdateInterval=1000*parseInt(document.getElementById("Refreshtime").value);
		MyDisplay.ChangeMonitoring($(this).is(':checked'));
		MyDisplay.RefreshView();
	});
	
	
	$("#buttonRefresh").button({text: false, icons: { primary: "ui-icon-refresh MyButtonStyle"}}).click(
			function() {
					MyDisplay.RefreshMonitor();
				});
	
	
	$("#Trending").button({text: false, icons: { primary: "ui-icon-image MyButtonStyle"}}).click(function() {
		MyDisplay.SetTrending($(this).is(':checked'), parseInt(document.getElementById("Trendlength").value));
		MyDisplay.RefreshView();
	});
	
	
    $('#Refreshtime').spinner({
        min: 1,
        max: 120,
        step: 1
    });

    $('#Trendlength').spinner({
        min: 10,
        max: 10000,
        step: 10
    });
	
    $('#Filesize').spinner({
        min: 100,
        max: 4000,
        step: 100
    });
    
    
	MyDisplay.RefreshView();
	
	
	$(document).tooltip();

});
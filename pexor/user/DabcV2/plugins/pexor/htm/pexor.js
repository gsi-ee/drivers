
var Pexor;
var MyDisplay;



////////////// State class reflecting remote state and commnication:
function PexorState() {
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
		if(state==true)
		this.fFileOpen =true;
		else
			this.fFileOpen =false;
		} else {
		console.log("UpdateFilestate failed.");
	}
	//console.log("UpdateFilestate with ok=%s, value=%s, fileopen=%s, typeofFileopen=%s", ok, state, this.fFileOpen, typeof(this.fFileOpen));
}

PexorState.prototype.Update= function(callback){
	var pthis = this;
	//this.DabcParameter("PexDevice/PexorAcquisitionRunning", pthis.UpdateRunstate);
	//this.DabcParameter("PexReadout/PexorFileOn", pthis.UpdateFilestate);	
	this.DabcParameter("PexDevice/PexorAcquisitionRunning", function(res,val) { pthis.UpdateRunstate(res,val); });
	this.DabcParameter("PexReadout/PexorFileOn",function(res,val) { pthis.UpdateFilestate(res,val); })
	callback();
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
		this.fTrendDa.force = true;
		this.fTrendDa.RegularCheck();
	} else {
		this.fGaugeEv.force = true;
		this.fGaugeEv.RegularCheck();
		this.fGaugeDa.force = true;
		this.fGaugeDa.RegularCheck();

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
			 $("label[for='buttonStartFile']").html("<span class=\"ui-button-text\"> Stop File </span>");		 
		} else {
			//console.log("RefreshView finds close file");
			$("#file_container").addClass("styleRed").removeClass("styleGreen");
			$("#buttonStartFile").prop('checked', false);
			$("label[for='buttonStartFile']").html("<span class=\"ui-button-text\"> Start File </span>");		
		}
	 
	 
	 if (this.fMonitoring) {
			$("#monitoring_container").addClass("styleGreen").removeClass("styleRed");
			 $("label[for='Monitoring']").html("<span class=\"ui-button-text\">  Stop Monitoring </span>");
			 
		} else {
			$("#monitoring_container").addClass("styleRed").removeClass("styleGreen");
			$("label[for='Monitoring']").html("<span class=\"ui-button-text\">  Start Monitoring </span>");
		}
	 
	 $("#Refreshtime").prop('disabled', this.fMonitoring);
	 
	 
	 if (this.fTrending) {
			 $("label[for='Trending']").html("<span class=\"ui-button-text\">  Show Rate Gauges </span>");
			 
		} else {
			$("label[for='Trending']").html("<span class=\"ui-button-text\">   Show Rate Trending </span>");
		}
	 $("#Trendlength").prop('disabled', this.fTrending);
	 
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
	
	$("#buttonStartAcquisition").button().click(
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

	$("#buttonStopAcquisition").button().click(
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
	$("#buttonInitAcquisition").button().click(function() {
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

	$("#buttonStartFile").button()
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
						 + "&FileSizeLimit=" + datafilelimit;

						Pexor.DabcCommand("PexReadout/StartFile", options,function(
								result) {
							MyDisplay.SetStatusMessage(result ? "Start File command sent."
									: "Start File FAILED.");
							if (result)
								{
								Pexor.fFileName = datafilename;
								}
							MyDisplay.RefreshMonitor();
							
							
						});
							}
						
						

					});



	$("#Monitoring").button().click(function() {		
		//MyDisplay.fUpdateInterval= Number($("#Refreshtime").value); // does not work?
		MyDisplay.fUpdateInterval=parseInt(document.getElementById("Refreshtime").value);
		MyDisplay.ChangeMonitoring($(this).is(':checked'));
		MyDisplay.RefreshView();
	});
	
	
	$("#buttonRefresh").button().click(
			function() {
					MyDisplay.RefreshMonitor();
				});
	
	
	$("#Trending").button().click(function() {
		MyDisplay.SetTrending($(this).is(':checked'), parseInt(document.getElementById("Trendlength").value));
		MyDisplay.RefreshView();
	});
	
	
	MyDisplay.RefreshView();
	
	
	$(document).tooltip();

});
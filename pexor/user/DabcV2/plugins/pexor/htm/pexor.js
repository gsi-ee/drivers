

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

/////////////// DISPLAY class to manage current view:
function PexorDisplay(state){
	this.fPexorState=state;
	this.fMonitoring=false;
	this.fUpdateTimer=null;
	this.fUpdateInterval=2000; // ms
	this.fGaugeEv = null;
	this.fGaugeDa = null;
	this.fLogDevice=null;
	this.fLogReadout=null;
}

// set up view elements of display:
PexorDisplay.prototype.BuildView = function(){
	
	
	
	this.fGaugeEv = new DABC.GaugeDrawElement();
	this.fGaugeEv.itemname = "/PEXOR/PexReadout/PexorEvents";
	this.fGaugeEv.CreateFrames($("#EvRateDisplay"));

	this.fGaugeDa = new DABC.GaugeDrawElement();
	this.fGaugeDa.itemname = "/PEXOR/PexReadout/PexorData";
	this.fGaugeDa.CreateFrames($("#DatRateDisplay"));

	
	this.fLogDevice= new DABC.LogDrawElement();
	this.fLogDevice.itemname = "/PEXOR/PexDevice/PexDevInfo";
	this.fLogDevice.EnableHistory(0);
	this.fLogDevice.CreateFrames($("#DeviceInfo"));
	
	this.fLogReadout= new DABC.LogDrawElement();
	this.fLogReadout.itemname = "/PEXOR/PexReadout/PexorInfo";
	this.fLogReadout.EnableHistory(0);
	this.fLogReadout.CreateFrames($("#ReadoutInfo"));
	
	// trending still does not work together with gauge elements?	
	// var trendEv=new DABC.RateHistoryDrawElement();
	// trendEv.itemname="/PEXOR/PexReadout/PexorEvents";
	// trendEv.CreateFrames($("#EvTrendDisplay"));
	// trendEv['monitoring']=true;
	// trendEv.RegularCheck();
	// setInterval(function(){ trendEv.RegularCheck();}, 2000);
	//	 
	// var trendDa=new DABC.RateHistoryDrawElement();
	// trendDa.itemname="/PEXOR/PexReadout/PexorData";
	// //trendDa.EnableHistory(100);
	// trendDa.CreateFrames($("#DatTrendDisplay"));
	// trendDa['monitoring']=true;
	//	 trendDa.RegularCheck();				 
	//	 setInterval(function(){ trendDa.RegularCheck();}, 2000);
	
}

PexorDisplay.prototype.RefreshMonitor = function(){
	
	this.fGaugeEv.RegularCheck();
	this.fGaugeDa.RegularCheck();
	this.fLogDevice.force=true;
	this.fLogDevice.RegularCheck();
	this.fLogReadout.force=true;
	this.fLogReadout.RegularCheck();
	
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





PexorDisplay.prototype.RefreshView = function(){
	
	 if (this.fPexorState.fRunning) {
			$("#daq_container").addClass("styleGreen").removeClass("styleRed");
		} else {
			$("#daq_container").addClass("styleRed").removeClass("styleGreen");
		}
	
	 if (this.fPexorState.fFileOpen) {
			$("#file_container").addClass("styleGreen").removeClass("styleRed");
		} else {
			$("#file_container").addClass("styleRed").removeClass("styleGreen");
		}
	 
	 
	 if (this.fMonitoring) {
			$("#monitoring_container").addClass("styleGreen").removeClass("styleRed");
		} else {
			$("#monitoring_container").addClass("styleRed").removeClass("styleGreen");
		}
	 
	
	this.RefreshMonitor();
	
};


PexorDisplay.prototype.SetStatusMessage= function(info) {
	var d = new Date();
	var txt = d.toLocaleString() + "  >" + info;
	document.getElementById("status_message").innerHTML = txt;
}

var Pexor;
var MyDisplay;



$(function() {

	Pexor = new PexorState();
	MyDisplay=new PexorDisplay(Pexor);
	MyDisplay.BuildView();
	
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
					if (result)
						Pexor.fRunning = true;
					// todo:
					MyDisplay.RefreshView();
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
					if (result)
						Pexor.fRunning = false;
					// todo:
					MyDisplay.RefreshView();
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
			MyDisplay.RefreshView();
			
		});

	});

	$("#buttonStartFile").button()
			.click(
					function() {
						var datafilename = prompt(
								"Please specify output file (*.lmd) ",
								Pexor.fFileName);
						// TODO: prompt for outputfile and file size limit
						if (!datafilename)
							return;
						var options = "FileName=" + datafilename
						// &FileSizeLimit=1000

						Pexor.DabcCommand("PexReadout/StartFile", options,function(
								result) {
							MyDisplay.SetStatusMessage(result ? "Start File command sent."
									: "Start File FAILED.");
							if (result)
								Pexor.fFileOpen = true;
							// todo: evaluate real state of file open/close
							MyDisplay.RefreshView();
						});


					});

	$("#buttonStopFile").button().click(
			function() {
				var requestmsg = "Really Stop writing output file "
						+ Pexor.fFileName + " ?";
				var response = confirm(requestmsg);
				if (!response)
					return;

				Pexor.DabcCommand("PexReadout/StopFile","",function(
						result) {
					MyDisplay.SetStatusMessage(result ? "Stop File command sent."
							: "Stop File FAILED.");
					if (result)
						Pexor.fFileOpen = false;
					// todo: evaluate real state of file open/close
					MyDisplay.RefreshView();
				});

			});

	$("#Monitoring").button().click(function() {
		MyDisplay.fMonitoring= $(this).is(':checked');
		//MyDisplay.fUpdateInterval= Number($("#Refreshtime").value);
		MyDisplay.fUpdateInterval=parseInt(document.getElementById("Refreshtime").value);
		
		$("#Refreshtime").prop('disabled', MyDisplay.fMonitoring);
		MyDisplay.ChangeMonitoring(MyDisplay.fMonitoring);
		MyDisplay.RefreshView();
	});
	
	
	MyDisplay.RefreshView();
	
	
	$(document).tooltip();

});
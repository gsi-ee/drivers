

function DabcCommand(cmd) {
	var xmlHttp = new XMLHttpRequest();
	var pre="/PEXOR/PexDevice/";
	var suf="/execute";
	var fullcom=pre+cmd+suf;
	console.log(fullcom);
	xmlHttp.open('GET', fullcom, true);
	xmlHttp.onreadystatechange = function() {
		
		if (xmlHttp.readyState == 4) {
			console.log("DabcCommand completed.");
			var reply = JSON.parse(xmlHttp.responseText);
			console.log("Reply= %s", reply);
		}
	}
	xmlHttp.send(null);
};



$(function() {

	
	$("#buttonStartAcquisition").button().click(function() {
		DabcCommand("StartAcquisition");
			});

	$("#buttonStopAcquisition").button().click(function() {
		DabcCommand("StopAcquisition");
			});
	$("#buttonInitAcquisition").button().click(function() {
		DabcCommand("InitAcquisition");
			});
	
	
	
	 var gaugeEv=new DABC.GaugeDrawElement();
	 gaugeEv.itemname="/PEXOR/PexReadout/PexorEvents";
	 gaugeEv.CreateFrames($("#EvRateDisplay"));
	 gaugeEv.RegularCheck();				 
	 setInterval(function(){ gaugeEv.RegularCheck();}, 2000);
	 
	 var gaugeDa=new DABC.GaugeDrawElement();
	 gaugeDa.itemname="/PEXOR/PexReadout/PexorData";
	 gaugeDa.CreateFrames($("#DatRateDisplay"));
	 gaugeDa.RegularCheck();				 
	 setInterval(function(){ gaugeDa.RegularCheck();}, 2000);
	
	 var trendEv=new DABC.RateHistoryDrawElement();
	 trendEv.itemname="/PEXOR/PexReadout/PexorEvents";
	 trendEv.CreateFrames($("#EvTrendDisplay"));
	 trendEv['monitoring']=true;
	 trendEv.RegularCheck();				 
	 setInterval(function(){ trendEv.RegularCheck();}, 2000);
	 
	 var trendDa=new DABC.RateHistoryDrawElement();
	 trendDa.itemname="/PEXOR/PexReadout/PexorData"; 
	 //trendDa.EnableHistory(100);
	 trendDa.CreateFrames($("#DatTrendDisplay"));
	 trendDa['monitoring']=true;
	 trendDa.RegularCheck();				 
	 setInterval(function(){ trendDa.RegularCheck();}, 2000);
	 
	
	$( document ).tooltip();

});
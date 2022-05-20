#!/bin/bash
# testscript to produce a pulse at io1 if nothing is connected..
WRSYS=/mbs/driv/white_rabbit/fallout/PCx86_Linux_OSV_4_9_0_64_Deb
PATH=$WRSYS/bin:$PATH
LD_LIBRARY_PATH=$WRSYS/lib:$LD_LIBRARY_PATH
#####
saft-io-ctl baseboard -n IO1 -o 1 -d 1;
sleep 1;
saft-io-ctl baseboard -n IO1 -o 1 -d 0;

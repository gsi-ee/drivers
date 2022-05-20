#!/bin/bash
WRSYS=/mbs/driv/white_rabbit/fallout/PCx86_Linux_OSV_4_9_0_64_Deb
export PATH=$WRSYS/bin:$WRSYS/sbin:$PATH
export LD_LIBRARY_PATH=$WRSYS/lib:$LD_LIBRARY_PATH
export SAFTLIB_LEAPSECONDSLIST=$WRSYS/share/saftlib/leap-seconds.list
# start saftd here as user ?
saftd baseboard:dev/wbm0 
#> /tmp/saftstart.log 2>&1 
echo saftd is started
sleep 1
saft-io-ctl baseboard -b -n IO1
saft-ecpu-ctl baseboard -c 0xfffe000000000001 0xffffffffffffffff 0 0 -d
echo eca is configured.
sleep 1
/bin/pidof -x  saftd | /usr/bin/xargs /bin/kill '-s 9'
echo saftd terminated again.

#!/bin/bash
WRSYS=/mbs/driv/white_rabbit/fallout/PCx86_Linux_OSV_4_9_0_64_Deb
export PATH=$WRSYS/bin:$WRSYS/sbin:$PATH
export LD_LIBRARY_PATH=$WRSYS/lib:$LD_LIBRARY_PATH
export SAFTLIB_LEAPSECONDSLIST=$WRSYS/share/saftlib/leap-seconds.list
#ECABASE=0x40000c0

ECA_QUEUE_POP_OWR=0x40000c4
#+0x04 
ECA_QUEUE_FLAGS_GET=0x40000c8
#+0x08  
ECA_QUEUE_DEADLINE_HI_GET=0x40000E8
# +0x28
ECA_QUEUE_DEADLINE_LO_GET=0x40000EC
# +0x2c

echo "flags before:"
eb-read dev/wbm0 ${ECA_QUEUE_FLAGS_GET}/4; 
echo "high timestamp:"
eb-read dev/wbm0 ${ECA_QUEUE_DEADLINE_HI_GET}/4; 
echo "low timestamp"
eb-read dev/wbm0 ${ECA_QUEUE_DEADLINE_LO_GET}/4; 
eb-write dev/wbm0 ${ECA_QUEUE_POP_OWR}/4 0xf; 
echo "flags after:"
eb-read dev/wbm0 ${ECA_QUEUE_FLAGS_GET}/4; 


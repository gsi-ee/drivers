#!/bin/bash
# example script to test sfp bus io with gosipcmd tool
# JAM (j.adamczewski@gsi.de) 23-05-2014
# note: tested with exploder 1
export PATH=$PWD/../bin:$PATH
SFP=0
SLAVE=0
MAXSLAVE=1
DATADEPTH=1024
let "REG_BUF0=0xFFFFD0"  
# base address for buffer 0 : 0x0000
let "REG_BUF1= 0xFFFFD4" 
# base address for buffer 1 : 0x20000
let "REG_SUBMEM_NUM=0xFFFFD8" 
# num of channels 8
let "REG_SUBMEM_OFF= 0xFFFFDC" 
#offset of channels 0x4000
let "REG_DATA_LEN=0xFFFFEC" 
errcount=0
echo starting gosipcmd test script...
echo initializing chain at sfp $SFP with $MAXSLAVE  slaves...
gosipcmd -i $SFP $MAXSLAVE
dbuf0=`gosipcmd -r $SFP $SLAVE $REG_BUF0`
echo read base address of buffer 0 $dbuf0
dbuf1=`gosipcmd -r $SFP $SLAVE $REG_BUF1`
echo read base address of buffer 1 $dbuf1
numsubs=`gosipcmd -r $SFP $SLAVE $REG_SUBMEM_NUM`
echo read number of submems  $numsubs
suboff=`gosipcmd -r $SFP $SLAVE $REG_SUBMEM_OFF`
echo read submem offset  $suboff
echo writing data depth $DATADEPTH ...
gosipcmd -w $SFP $SLAVE $REG_DATA_LEN $DATADEPTH
echo starting token memory write test...
numwords=$DATADEPTH/4
for submem in `seq $numsubs`
do
    subix=$(($submem -1))
    submembase0=$(($dbuf0 + $subix * $suboff))    
    for ((a=0; a < numwords ; a++))  # Double parentheses, and naked "LIMIT"
    do
        let address="$submembase0 + $(($a *4))"
        gosipcmd -w $SFP $SLAVE $address $a
   done  
done

echo starting read and verify test...
for submem in `seq $numsubs`
do
     subix=$(($submem -1))
    submembase0=$(($dbuf0 + $subix * $suboff))    
    for ((a=0; a < numwords ; a++))  # Double parentheses, and naked "LIMIT"
    do
        let address="$submembase0 + $(($a *4))"
        value=`gosipcmd -r $SFP $SLAVE $address`
        if [ "$a" -ne "$value" ]
        then
            echo "error: read back value $value is not equal to written value $a"
            ((errcount++))
        fi
   done  
done
echo "gosipcmd io test finished with $errcount errors."
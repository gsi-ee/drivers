#!/bin/bash
#
# N.Kurz, EE, GSI, 17-Jun-2015
#

POTVAL=$(($3))

if [ $1 -gt 3 ] || [ $1 -lt 0 ]
then
 echo
 echo "ERROR>> SFP id out of range [0-3]"
 echo "Synopsis: sh set_dac_x.sh SFP_ID SLAVE_ID[hex] POT_VAL[dec or hex]"
  echo "exiting.."
 exit
fi

if [ $POTVAL -gt 255 ] || [ $POTVAL -lt 0 ]
then
 echo
 echo "ERROR>> potentiometer input value out of range [0-255 or 0-0xff]"
 echo "Synopsis: sh set_dac_x.sh SFP_ID SLAVE_ID[hex] POT_VAL[dec or hex]"
 echo "exiting.."
 exit
fi

# enable i2c
gosipcmd -d -x -w $1 $2 208010  1000080
gosipcmd -d -x -w $1 $2 208010  2000020

# set mcp443x/5x chip, chip id 0 (has 4 channels, febex channel 0-3)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x62580000+$3))"|bc`   # pexor chan 0,   least significant byte: 80: middle, max: ff, min:0 (0:  shift to max adc value)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x62581000+$3))"|bc`   # pexor chan 1,   least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x62586000+$3))"|bc`   # pexor chan 2,   least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x62587000+$3))"|bc`   # pexor chan 3,   least significant byte: 80: middle, max: ff, min:0

# set mcp443x/5x chip, chip id 1 (has 4 channels, febex channel 4-7)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625a0000+$3))"|bc`   # pexor chan 4,   least significant byte: 80: middle, max: ff, min:0 (0:  shift to max adc value)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625a1000+$3))"|bc`   # pexor chan 5,   least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625a6000+$3))"|bc`   # pexor chan 6,   least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625a7000+$3))"|bc`   # pexor chan 7,   least significant byte: 80: middle, max: ff, min:0

# set mcp443x/5x chip, chip id 2 (has 4 channels, febex channel 8-11)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625c0000+$3))"|bc`   # pexor chan 8,   least significant byte: 80: middle, max: ff, min:0 (0:  shift to max adc value)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625c1000+$3))"|bc`   # pexor chan 9,   least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625c6000+$3))"|bc`   # pexor chan 10,  least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625c7000+$3))"|bc`   # pexor chan 11,  least significant byte: 80: middle, max: ff, min:0

# set mcp443x/5x chip, chip id 3 (has 4 channels, febex channel 12-15)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625e0000+$3))"|bc`   # pexor chan 12,  least significant byte: 80: middle, max: ff, min:0 (0:  shift to max adc value)
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625e1000+$3))"|bc`   # pexor chan 13,  least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625e6000+$3))"|bc`   # pexor chan 14,  least significant byte: 80: middle, max: ff, min:0
gosipcmd -d -x -w $1 $2 208010 `expr "obase=16; $((0x625e7000+$3))"|bc`   # pexor chan 15,  least significant byte: 80: middle, max: ff, min:0


# set digital potentiomrter to move febex baseline 
gosipcmd -d -x -w $1 $2 208010 625841ff   # activate setting febex channel 0, 1
gosipcmd -d -x -w $1 $2 208010 6258a1ff   # activate setting febex channel 2, 3
gosipcmd -d -x -w $1 $2 208010 625a41ff   # activate setting febex channel 4, 5
gosipcmd -d -x -w $1 $2 208010 625aa1ff   # activate setting febex channel 6, 7
gosipcmd -d -x -w $1 $2 208010 625c41ff   # activate setting febex channel 8, 9
gosipcmd -d -x -w $1 $2 208010 625ca1ff   # activate setting febex channel 10, 11
gosipcmd -d -x -w $1 $2 208010 625e41ff   # activate setting febex channel 12, 13
gosipcmd -d -x -w $1 $2 208010 625ea1ff   # activate setting febex channel 14, 15

# disable i2c
gosipcmd -d -x -w $1 $2 208010  1000000
exit

# read back digital potentiometer to move febex baseline 
gosipcmd -d -x -w $1 $2 208010 e2580c00   # read request for data (i2c)(febex channel 0)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e2581c00   # read request for data (i2c)(febex channel 1)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e2586c00   # read request for data (i2c)(febex channel 2)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e2587c00   # read request for data (i2c)(febex channel 3)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           

# read back digital potentiometer to move febex baseline 
gosipcmd -d -x -w $1 $2 208010 e25a0c00   # read request for data (i2c)(febex channel 4)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25a1c00   # read request for data (i2c)(febex channel 5)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25a6c00   # read request for data (i2c)(febex channel 6)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25a7c00   # read request for data (i2c)(febex channel 7)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           

# read back digital potentiometer to move febex baseline 
gosipcmd -d -x -w $1 $2 208010 e25c0c00   # read request for data (i2c)(febex channel 8)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25c1c00   # read request for data (i2c)(febex channel 9)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25c6c00   # read request for data (i2c)(febex channel 10)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25c7c00   # read request for data (i2c)(febex channel 11)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           

# read back digital potentiometer to move febex baseline 
gosipcmd -d -x -w $1 $2 208010 e25e0c00   # read request for data (i2c)(febex channel 12)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25e1c00   # read request for data (i2c)(febex channel 13)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25e6c00   # read request for data (i2c)(febex channel 14)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           
gosipcmd -d -x -w $1 $2 208010 e25e7c00   # read request for data (i2c)(febex channel 15)
gosipcmd -d -x -w $1 $2 208010 86000000   #   
gosipcmd -d -x -r $1 $2 208020 1           

# disable i2c
#gosipcmd -d -x -w $1 $2 208010  1000000

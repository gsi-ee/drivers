#! /bin/bash
echo "load_kinpex..."

sudo /sbin/insmod pexor.ko
sudo /bin/chmod g+wr  /dev/kinpex*
sudo /bin/chgrp rz /dev/kinpex*
ln -s /dev/kinpex-0 /dev/pexor-0
echo "                 done."

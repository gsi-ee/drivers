#! /bin/bash
echo "load_pexor..."
sudo /sbin/insmod pexor.ko
sudo /bin/chmod g+wr  /dev/pexor*
sudo /bin/chgrp users /dev/pexor*
echo "                 done."

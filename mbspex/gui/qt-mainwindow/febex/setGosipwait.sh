#!/bin/bash
echo "$1" > /sys/class/mbspex/*/gosipbuswait
echo set gosip bus wait time to $1 useconds

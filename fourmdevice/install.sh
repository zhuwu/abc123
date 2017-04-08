#!/bin/bash

make
insmod fourmdevice.ko
cd /dev
mknod fourmdevice c 61 1
chmod 666 fourmdevice

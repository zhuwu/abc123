#!/bin/bash


insmod chardevice.ko
cd /dev
mknod chardevice c 61 1
chmod 666 chardevice

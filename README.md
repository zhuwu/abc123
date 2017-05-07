# Hello World Linux Device

This repository is the course project of NUS CS5250 Advanced Operating Systems.

* Directory `helloworld` is the most basic hello world device. 
* Directory `device` is a character device which can hold one byte data.
* Directory `fourmdevice` is a character device which can hold 4MiB data. The device supports `lseek` and some dummy control via `ioctl`.

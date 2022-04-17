/*
File: devices.h
Description: Header file for device enumeration, devices.cpp
Author: Daniel Limanowski
*/

// Function Prototypes
HRESULT FindDevices(REFGUID category, IEnumMoniker **ppEnum);
void GetDeviceInfo(IEnumMoniker *pEnum);
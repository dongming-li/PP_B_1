/*
File: devices.cpp
Description: Enumerates information about devices on a host
Author: Daniel Limanowski
*/

#include <windows.h>
#include <dshow.h>
#include <iostream>

#include "devices.h"

#pragma comment(lib, "strmiids")

/**
Enumerates devices based on the inputted category
Some possible categories are audio devices and video devices
**/
HRESULT FindDevices(REFGUID category, IEnumMoniker **ppEnum) {
	ICreateDevEnum *pDevEnum;
	HRESULT res = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(res)) {
		res = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);

		if (res == S_FALSE) {
			//nothing found in the supplied category
			res = VFW_E_NOT_FOUND;
		}

		pDevEnum->Release();
	}
	return res;
}

/**
Get the name and description of a device
**/
void GetDeviceInfo(IEnumMoniker *pEnum) {
	IMoniker *pMoniker = NULL;

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
		IPropertyBag *pPropBag;
		HRESULT res = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));

		if (FAILED(res)) {
			pMoniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		//get name of device and its description
		res = pPropBag->Read(L"Description", &var, 0);
		if (FAILED(res)) {
			res = pPropBag->Read(L"FriendlyName", &var, 0);
		}
		
		if (SUCCEEDED(res)) {
			printf("%S\n", var.bstrVal);
			VariantClear(&var);
		}

		res = pPropBag->Write(L"FriendlyName", &var);

		//this section (WaveInID) applies to audio devices only
		res = pPropBag->Read(L"WaveInID", &var, 0);
		if (SUCCEEDED(res)) {
			printf("WaveIn ID: %d\n", var.lVal);
			VariantClear(&var);
		}

		pPropBag->Release();
		pMoniker->Release();
	}
}


/*
Main function for debugging purposes. Will show a console window with video devices found

NOTE: Uncomment if testing with just this file
*/
/*
void main() {
	std::cout << "(Video) Devices found:" << std::endl;
	HRESULT res = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(res)) {
		IEnumMoniker *pEnum;
		res = FindDevices(CLSID_VideoInputDeviceCategory, &pEnum);

		if (SUCCEEDED(res)) {
			GetDeviceInfo(pEnum);
			pEnum->Release();
		}
	}
	CoUninitialize();

	std::cout << "\r\n\r\nHit enter to exit" << std::endl;
	//make sure to keep the console window open so as to see what was found
	std::cin.ignore();
}
*/
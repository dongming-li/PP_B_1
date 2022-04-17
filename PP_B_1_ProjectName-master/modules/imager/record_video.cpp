/*
File: preview_video.cpp
Description: Shows USB camera video preview in a pop-up window
*/
#include <windows.h>
#include <dshow.h>
#include <strsafe.h>
#include <stdio.h>
#include <cstdlib> //Sleep() - function that waits specified amount of milliseconds

#include "record_video.h"

//NEED THIS FOR STATIC LINKING OF LIBRARIES
#pragma comment(lib,"Strmiids.lib")

#define _WIN32_WINNT 0x0500 //Windows OS version constant, set to Windows 2000 for minimum compatability
//See: https://msdn.microsoft.com/en-us/library/6sehtctf.aspx for more

// Global DirectShow variables
IGraphBuilder * g_pGraph = NULL;
ICaptureGraphBuilder2 * g_pCapture = NULL;
IBaseFilter * g_pMux = NULL; // for recording to avi
IVideoWindow * g_pVW = NULL;
IMediaControl * g_pMC = NULL;
IMediaEvent * g_pME = NULL;

HRESULT CaptureVideo()
{
	HRESULT hr;
	IBaseFilter *pSrcFilter = NULL;
	
	//For recording video for 5 seconds
	REFERENCE_TIME rtStart = 10000000, rtStop = 60000000;
	const WORD wStartCookie = 1, wStopCookie = 2;

	// Get DirectShow interfaces
	hr = GetInterfaces();
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
		return hr;
	}

	// Attach the filter graph to the capture graph
	hr = g_pCapture->SetFiltergraph(g_pGraph);
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
		return hr;
	}

	// Use the system device enumerator and class enumerator to find
	// a video capture/preview device, such as a desktop USB video camera.
	hr = FindCaptureDevice(&pSrcFilter);
	if (FAILED(hr))
	{
		// Don't display a message because FindCaptureDevice will handle it
		return hr;
	}

	// Add Capture filter to our graph.
	hr = g_pGraph->AddFilter(pSrcFilter, L"Video Capture");
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
			TEXT("If you have a working video capture device, please make sure\r\n")
			TEXT("that it is connected and is not being used by another application.\r\n\r\n")
			TEXT("The program will now close."), hr);
		pSrcFilter->Release();
		return hr;
	}

	// Set the output filename and type
	//The directory path of the VFS: L"\\\\.\\Hd1\\\\filename_here.avi"
	hr = g_pCapture->SetOutputFileName(&MEDIASUBTYPE_Avi, L"C:\\\\vid_cap.avi", &g_pMux, NULL);
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't set output filename.  hr=0x%x\r\n", hr));
		pSrcFilter->Release();
		return hr;
	}

	// Only allow recording for 5 seconds, 1 second after the stream begins rendering
	hr = g_pCapture->ControlStream(&PIN_CATEGORY_CAPTURE, NULL, NULL, &rtStart, &rtStop, wStartCookie, wStopCookie);
	
	// Render the capture pin on the video capture filter
	// Use this instead of g_pGraph->RenderFile
	hr = g_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSrcFilter, NULL, g_pMux);
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't render the video capture stream.  hr=0x%x\r\n")
			TEXT("The capture device may already be in use by another application.\r\n\r\n")
			TEXT("The program will now close."), hr);
		pSrcFilter->Release();
		return hr;
	}

	g_pMux->Release();

	// Now that the filter has been added to the graph and we have
	// rendered its stream, we can release this reference to the filter.
	pSrcFilter->Release();

	// Start recording the video data to an avi
	hr = g_pMC->Run();
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
		return hr;
	}

	return S_OK;
}


HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter)
{
	HRESULT hr = S_OK;
	IBaseFilter * pSrc = NULL;
	IMoniker* pMoniker = NULL;
	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pClassEnum = NULL;

	if (!ppSrcFilter)
	{
		return E_POINTER;
	}

	// Create the system device enumerator
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC,
		IID_ICreateDevEnum, (void **)&pDevEnum);
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't create system enumerator!  hr=0x%x"), hr);
	}

	// Create an enumerator for the video capture devices

	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
		if (FAILED(hr))
		{
			Msg(TEXT("Couldn't create class enumerator!  hr=0x%x"), hr);
		}
	}

	if (SUCCEEDED(hr))
	{
		// If there are no enumerators for the requested type, then 
		// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
		if (pClassEnum == NULL)
		{
			MessageBox(NULL, TEXT("No video capture device was detected.\r\n\r\n")
				TEXT("This program requires a video capture device, such as a USB WebCam,\r\n")
				TEXT("to be installed and working properly.  The program will now close."),
				TEXT("No Video Capture Hardware"), MB_OK | MB_ICONINFORMATION);
			hr = E_FAIL;
		}
	}

	// Use the first video capture device on the device list.
	// Note that if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure).  Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.

	if (SUCCEEDED(hr))
	{
		hr = pClassEnum->Next(1, &pMoniker, NULL);
		if (hr == S_FALSE)
		{
			Msg(TEXT("Unable to access video capture device!"));
			hr = E_FAIL;
		}
	}

	if (SUCCEEDED(hr))
	{
		// Bind Moniker to a filter object
		hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&pSrc);
		if (FAILED(hr))
		{
			Msg(TEXT("Couldn't bind moniker to filter object!  hr=0x%x"), hr);
		}
	}

	// Copy the found filter pointer to the output parameter.
	if (SUCCEEDED(hr))
	{
		*ppSrcFilter = pSrc;
		(*ppSrcFilter)->AddRef();
	}

	SAFE_RELEASE(pSrc);
	SAFE_RELEASE(pMoniker);
	SAFE_RELEASE(pDevEnum);
	SAFE_RELEASE(pClassEnum);

	return hr;
}

HRESULT GetInterfaces(void)
{
	HRESULT hr;

	// Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
		IID_IGraphBuilder, (void **)&g_pGraph);
	if (FAILED(hr))
		return hr;

	// Create the capture graph builder
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
		IID_ICaptureGraphBuilder2, (void **)&g_pCapture);
	if (FAILED(hr))
		return hr;

	// Obtain interfaces for media control and Video Window
	hr = g_pGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&g_pMC);
	if (FAILED(hr))
		return hr;

	hr = g_pGraph->QueryInterface(IID_IVideoWindow, (LPVOID *)&g_pVW);
	if (FAILED(hr))
		return hr;

	hr = g_pGraph->QueryInterface(IID_IMediaEventEx, (LPVOID *)&g_pME);
	if (FAILED(hr))
		return hr;

	return hr;
}

void CloseInterfaces(void)
{
	// Stop previewing data
	if (g_pMC)
		g_pMC->StopWhenReady();
}

void Msg(TCHAR *szFormat, ...)
{
	TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
	const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
	const int LASTCHAR = NUMCHARS - 1;

	// Format the input string
	va_list pArgs;
	va_start(pArgs, szFormat);

	// Use a bounded buffer size to prevent buffer overruns.  Limit count to
	// character size minus one to allow for a NULL terminating character.
	(void)StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
	va_end(pArgs);

	// Ensure that the formatted string is NULL-terminated
	szBuffer[LASTCHAR] = TEXT('\0');

	MessageBox(NULL, szBuffer, TEXT("VidCap Message"), MB_OK | MB_ICONERROR);
}


int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg = { 0 };

	// Initialize COM
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		Msg(TEXT("CoInitialize Failed!\r\n"));
		exit(1);
	}

	HRESULT hr;

	// Create DirectShow graph and start capturing video
	hr = CaptureVideo();
	if (FAILED(hr))
	{
		CloseInterfaces();
	}

	Sleep(1000 * VIDEO_LENGTH_SECONDS);

	CloseInterfaces();

	// Release COM
	CoUninitialize();

	return (int)msg.wParam;
}
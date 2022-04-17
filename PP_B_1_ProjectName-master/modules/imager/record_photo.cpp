/*
File: record_photo.cpp
Description: Takes a sample from webcam video stream, converts it to
RGB888 and saves it as a proper bitmap image
*/
#include <windows.h>
#include <dshow.h>
#include <strsafe.h>
#include <stdio.h>
#include <cstdlib> //Sleep() - function that waits specified amount of milliseconds
#include <qedit.h>
#include "record_photo.h"

//NEED THIS FOR STATIC LINKING OF LIBRARIES
#pragma comment(lib,"Strmiids.lib")

#define _WIN32_WINNT 0x0501 //Windows OS version constant, set to Windows XP for minimum compatability (restricted by sample grabber)
//See: https://msdn.microsoft.com/en-us/library/6sehtctf.aspx for more

// Directshow global variables
IGraphBuilder *g_pGraph = NULL;
ICaptureGraphBuilder2 *g_pCapture = NULL;
IMediaControl *g_pControl = NULL;
IMediaEvent *g_pEvent = NULL;

HRESULT CapturePhoto(LPCSTR picFileName)
{
	HRESULT hr;
	IBaseFilter *pSrcFilter = NULL; // will be the capture device
	BYTE *pBuffer = NULL;

	// Get DirectShow interfaces
	hr = GetInterfaces();
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to get directshow interfaces!  hr=0x%x"), hr);
		return hr;
	}

	// Attach the filter graph to the capture graph
	hr = g_pCapture->SetFiltergraph(g_pGraph);
	if (FAILED(hr))
	{
		Msg(TEXT("Failed to set capture's filter graph!  hr=0x%x"), hr);
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
	hr = g_pGraph->AddFilter(pSrcFilter, L"Camera");
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
			TEXT("If you have a working video capture device, please make sure\r\n")
			TEXT("that it is connected and is not being used by another application.\r\n\r\n")
			TEXT("The program will now close."), hr);
		pSrcFilter->Release();
		return hr;
	}


	// Now that the filter has been added to the graph and we have
	// rendered its stream, we can release this reference to the filter.

	// Add the Sample Grabber filter to the graph.
	IBaseFilter *pGrabberFilter;
	ISampleGrabber *pGrabber;

	hr = CoCreateInstance(
		CLSID_SampleGrabber,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,
		(void**)&pGrabberFilter
	);

	hr = g_pGraph->AddFilter(pGrabberFilter, L"Sample Grabber");
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't add sample grabber filter to graph!  hr=0x%x"), hr);
	}

	hr = pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't query Sample Grabber Filter interface!  hr=0x%x"), hr);
	}

	// Add the Null Renderer filter to the graph.
	IBaseFilter *pNullFilter;

	hr = CoCreateInstance(
		CLSID_NullRenderer,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IBaseFilter,
		(void**)&pNullFilter
	);

	hr = g_pGraph->AddFilter(pNullFilter, L"NullRender");

	hr = pGrabber->SetOneShot(TRUE); //Stops after receiving first sample
	hr = pGrabber->SetBufferSamples(TRUE); // Stores copy of sample

	hr = g_pCapture->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pSrcFilter, pGrabberFilter, pNullFilter);
	
	// Start the media control graph
	hr = g_pControl->Run();
	if (FAILED(hr))
	{
		Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
		return hr;
	}

	// Now wait for and save the first captured sample
	long evCode;
	hr = g_pEvent->WaitForCompletion(INFINITE, &evCode);

	//To determine the size of the array that is needed for the sample located in the buffer, 
	//first call GetCurrentBuffer with a NULL pointer for the array address.
	long cbBuffer;
	hr = pGrabber->GetCurrentBuffer(&cbBuffer, NULL);

	//Then allocate the array and call the method a second time to copy the buffer.
	pBuffer = (BYTE*)CoTaskMemAlloc(cbBuffer);
	if (!pBuffer)
	{
		hr = E_OUTOFMEMORY;
		Msg(TEXT("Out of memory!  hr=0x%x"), hr);

	}

	//then call GetCurrentBuffer() a second time to copy the buffer into our array
	hr = pGrabber->GetCurrentBuffer(&cbBuffer, (long*)pBuffer);
	
	// Set the media type
	AM_MEDIA_TYPE media_type;
	media_type.majortype = MEDIATYPE_Video; // only accept video
	SecureZeroMemory(&media_type, sizeof(media_type));
	// The rest of the values for AM_MEDIA_TYPE will now be filled in by
	// the camera's real data
	hr = pGrabber->GetConnectedMediaType(&media_type);

	if (hr != S_OK) {
		//ERROR!
		Msg(TEXT("Unable to Get Connected Media Type!"));
	}

	// Examine format block
	if ((media_type.formattype == FORMAT_VideoInfo) &&
		(media_type.cbFormat >= sizeof(VIDEOINFOHEADER)) &&
		(media_type.pbFormat != NULL))
	{
		VIDEOINFOHEADER *pVIH = (VIDEOINFOHEADER*)media_type.pbFormat;

		//if all succeeds, then we can go to the method to write a bitmap file
		hr = WriteBitmap(picFileName, &pVIH->bmiHeader,
			media_type.cbFormat - SIZE_PREHEADER, pBuffer, cbBuffer);
	}
	else
	{
		// Invalid format.
		hr = VFW_E_INVALIDMEDIATYPE;
	}

	return S_OK;
}

HRESULT FindCaptureDevice(IBaseFilter **ppSrcFilter)
{
	HRESULT hr;
	IBaseFilter *pSrc = NULL;
	IMoniker *pMoniker = NULL;
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

	// Obtain interfaces for media control (pControl) & media events (pEvent)
	hr = g_pGraph->QueryInterface(IID_IMediaControl, (LPVOID *)&g_pControl);
	if (FAILED(hr))
		return hr;
	hr = g_pGraph->QueryInterface(IID_IMediaEventEx, (LPVOID *)&g_pEvent);
	if (FAILED(hr))
		return hr;

	return hr;
}

void CloseInterfaces(void)
{
	// Stop previewing data
	if (g_pControl)
		g_pControl->StopWhenReady();

	// Release DirectShow interfaces
	SAFE_RELEASE(g_pControl);
	SAFE_RELEASE(g_pEvent);
	SAFE_RELEASE(g_pGraph);
	SAFE_RELEASE(g_pCapture);
}


// Writes a bitmap file
//  pszFileName:  Output file name.
//  pBMI:         Bitmap format information
//  cbBMI:        Size of the BITMAPINFOHEADER
//  pData:        Pointer to the bitmap bits
//  cbData        Size of the bitmap, in bytes
HRESULT WriteBitmap(LPCSTR pszFileName, BITMAPINFOHEADER *pBMI, size_t cbBMI, BYTE *pData, size_t cbData) 
{
	// Declare Bitmap (.bmp) file structure
	BITMAPFILEHEADER bmpHFile;
	BITMAPINFOHEADER bmpHInfo;
	// Initializing to zero:
	SecureZeroMemory(&bmpHFile, sizeof(BITMAPFILEHEADER));
	SecureZeroMemory(&bmpHInfo, sizeof(BITMAPINFOHEADER));

	//
	// Setting new bitmap info header for our image file:
	//
	bmpHInfo.biSize = sizeof(BITMAPINFOHEADER);
	bmpHInfo.biWidth = pBMI->biWidth;
	bmpHInfo.biHeight = pBMI->biHeight;
	bmpHInfo.biPlanes = 1;                  //One "bitplane"
	bmpHInfo.biCompression = BI_RGB;
	bmpHInfo.biBitCount = 24;               // RGB888 or 24 bit rgb
	bmpHInfo.biSizeImage = 0;               // Can set to 0 b/c we are using BI_RGB
	bmpHInfo.biXPelsPerMeter = 0;           // DPI - we don't care about this, won't use it
	bmpHInfo.biYPelsPerMeter = 0;
	bmpHInfo.biClrUsed = 0;                 // RGB mode and have no palette
	bmpHInfo.biClrImportant = 0;            // all colors important

	//
	// Setting bitmap file header:
	//
	bmpHFile.bfType = 'MB'; // Little Endian for BM...bitmap
	bmpHFile.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpHInfo.biSizeImage;
	bmpHFile.bfReserved1 = 0;
	bmpHFile.bfReserved2 = 0;
	bmpHFile.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(bmpHInfo); //bfOffBits = size of file head, size of info head, size of color pallete, if applicable

	//
    // Create the file for writing the bitmap data to:
	//
	HANDLE hFile = CreateFile(pszFileName, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); //CREATE_ALWAYS overwrites the existing file if one exists
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// 
	// Manipulate the raw image data now
	// 
	BYTE* image_data = NULL;
	DWORD rgbSize = NULL;

	// If necessary, convert YU2 stream to RGB buffer:
	if (pBMI->biCompression == 0x32595559) { // 0x32595559 == YUY2 (FOURCC)
		image_data = ConvertBuffer_YUY2_RGB(pData, bmpHInfo.biWidth, bmpHInfo.biHeight);
	}

	// Convert the RGB stream to BMP buffer:
	BYTE* bmpData = ConvertBuffer_RGB_BMP(image_data, bmpHInfo.biWidth, bmpHInfo.biHeight, &rgbSize);
	
	DWORD* sizePtr = &rgbSize; // pointing to the updated size from bmp buffer

	//
	// Write the file
	//
	DWORD cbWritten = 0; // represents bytes written by WriteFile() <- helps with debugging, can set NULL

	// First write the file header of the bitmap file:
	BOOL result = WriteFile(hFile, &bmpHFile, sizeof(BITMAPFILEHEADER), &cbWritten, NULL);
	if (result)
	{	
		// If the file head write worked, next write info header:
		result = WriteFile(hFile, &bmpHInfo, sizeof(BITMAPINFOHEADER), &cbWritten, NULL);
	}
	// If info head write worked, next write the image data:
	if (result)
	{
		result = WriteFile(hFile, bmpData, *sizePtr, &cbWritten, NULL);
	}

	//
	// Clean up
	//
	HRESULT hr = result ? S_OK : HRESULT_FROM_WIN32(GetLastError());

	CloseHandle(hFile);
	//De-allocate pointers
	free(image_data);

	return hr;
}

BYTE* ConvertBuffer_RGB_BMP(BYTE* rgbBuffer, int width, int height, DWORD* size) 
{
	// some exception checking:
	if ((rgbBuffer == NULL) || (width < 1) || (height < 1)) {
		return NULL;
	}

	int pad = 0;
	int scanlineBytes = width * 3;
	while ((scanlineBytes + pad) % 4 != 0) {
		pad++;
	}

	// Padded scanline width:
	int psw = scanlineBytes + pad;

	DWORD calc_size = height * psw;
	*size = calc_size; // size of BMP-padded buffer

	BYTE* bmpBuffer = new BYTE[*size];
	SecureZeroMemory(bmpBuffer, *size); // Allocate array, put in zeros
	BYTE* retAddr = bmpBuffer; // returning the bmp buffer at the beginning address

	long bufferPos = 0;
	long currPos = 0;
	for (int y = 0; y < height; y++) { // Loop the scanline 
		for (int x = 0; x < width * 3; x+=3) { // Loop the columns
			bufferPos = y * 3 * width + x; // the position in the rgb (original) buffer
			currPos = (height - y - 1) * psw + x; // position in bmp (final) buffer
			bmpBuffer[currPos] = rgbBuffer[bufferPos + 2]; // allows us to swap the red (r) and blue (b) values
			bmpBuffer[currPos + 1] = rgbBuffer[bufferPos + 1]; // Green (G) remains in place
			bmpBuffer[currPos + 2] = rgbBuffer[bufferPos]; // swap Blue (B) and Red (R)		
		}
	}
	return retAddr;
}

//Outputs a pure RGB buffer (ie., (R0, G0, B0, R1, G1, B1, ... , Rn, Gn, Bn) <- RGB888 (24 bit RGB)
// where n is total number of pixels in the image (width * height)
BYTE* ConvertBuffer_YUY2_RGB(BYTE* yuy2Buffer, int width, int height) 
{
	// some exception checking:
	if ((yuy2Buffer == NULL) || (width < 1) || (height < 1)) {
		return NULL;
	}

	unsigned int newSize = width * height * 3; // total num of pixels times
											   // red value, green value, blue value for each pixel
	BYTE* rgbBuffer = new BYTE[newSize];
	SecureZeroMemory(rgbBuffer, newSize); // Securely allocate the array with zeroes

	BYTE* retAddr = rgbBuffer; // we need to return the address of the beginning of the rgb buffer
	
	// Extracting 2 pixels of RGB per iteration so we need (totalPixels / 2) iterations
	for (int i = 0; i < (height * width) / 2; i++)
	{
		// YUY2 is a version of YUV4:2:2 and is in the following format:
		// | Y0 | U0 | Y1 | V0 | = 4 bytes = 2 pixels
		// Pixel 1 = (Y0, U0, V0) ... Pixel 2 = (Y1, U0, V0) ...
		// Output after conversion will then be: (R0, G0, B0, R1, G1, B1, ...)
		int y0 = yuy2Buffer[0];
		int u = yuy2Buffer[1];
		int y1 = yuy2Buffer[2];
		int v = yuy2Buffer[3];

		// The below formulas obtained from Microsoft and generally accepted by pros:
		// ~ first pixel ~
		// We subtract 16 and 128 from c, (d, e) respectively for "swing" (8 -> 16 bit) 
		// see more on this ^ here: https://en.wikipedia.org/wiki/YUV
		int c = y0 - 16;
		int d = u - 128;
		int e = v - 128;
		rgbBuffer[0] = clip((298 * c + 409 * e + 128) >> 8); // red
		rgbBuffer[1] = clip((298 * c - 100 * d - 208 * e + 128) >> 8); // green
		rgbBuffer[2] = clip((298 * c + 516 * d + 128) >> 8); // blue

		// ~ second pixel ~
		c = y1 - 16; // d and e remain the same for second pixel, only c (luma) changes
		rgbBuffer[3] = clip((298 * c + 409 * e + 128) >> 8); // red
		rgbBuffer[4] = clip((298 * c - 100 * d - 208 * e + 128) >> 8); // green
		rgbBuffer[5] = clip((298 * c + 516 * d + 128) >> 8); // blue
		
		// Move pointers to obtain next 4 bytes of YUY2 data
		yuy2Buffer += 4;
		rgbBuffer += 6;
	}

	return retAddr;
}

// RGB values must be between 0 and 255, exclusive
// This function ensures no values beyond the min/max
int clip(int in) 
{
	int clipped = in;

	if (in < 0) {
		clipped = 0;
	}
	else if (in > 255) {
		clipped = 255;
	}

	return clipped;
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

	MessageBox(NULL, szBuffer, TEXT("Imager Message"), MB_OK | MB_ICONERROR);
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

	// Create DirectShow graph to capture photo
	hr = CapturePhoto("\\\\.\\Hd1\\\\img_cap.bmp");
	//hr = CapturePhoto("img_cap.bmp"); // saves in the imager folder of repo
	if (FAILED(hr))
	{
		CloseInterfaces();
	}

	Sleep(1000); //give the video stream one second to capture photo

	CloseInterfaces();

	// Release COM
	CoUninitialize();

	return (int)msg.wParam;
}
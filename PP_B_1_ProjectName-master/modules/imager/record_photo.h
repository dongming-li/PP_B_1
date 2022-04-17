/*
File: record_photo.h
Description: Header file for image capture program, record_photo.cpp
*/

// Function prototypes
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow);
HRESULT GetInterfaces(void);
HRESULT CapturePhoto(LPCSTR picFileName);
HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter);
void Msg(TCHAR *szFormat, ...);
void CloseInterfaces(void);
HRESULT WriteBitmap(LPCSTR pszFileName, BITMAPINFOHEADER *pBMI, size_t cbBMI, BYTE *pData, size_t cbData);
BYTE* ConvertBuffer_YUY2_RGB(BYTE* yuy2Buffer, int width, int height);
BYTE* ConvertBuffer_RGB_BMP(BYTE* rgbBuffer, int width, int height, DWORD* size);
int clip(int in);
// end prototypes

// Macros
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }
/*
File: record_video.h
Description: Header file for video recorder, record_video.c++
*/

// Function prototypes
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow);
HRESULT GetInterfaces(void);
HRESULT CaptureVideo();
HRESULT FindCaptureDevice(IBaseFilter ** ppSrcFilter);
void Msg(TCHAR *szFormat, ...);
void CloseInterfaces(void);
// End Function prototypes

// Macros
#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

// Constants
#define DEFAULT_VIDEO_WIDTH     (1920 / 2) // half of 1080p HD
#define DEFAULT_VIDEO_HEIGHT    (1080 / 2) // half of 1080p HD

#define VIDEO_LENGTH_SECONDS 5 //length of the video recorded in seconds
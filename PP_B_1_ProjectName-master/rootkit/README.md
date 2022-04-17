# INTRACTABLEGIRAFFE ROOTKIT

The INTRACETABLEGIRAFFEE rootkit ("IG") is a component of a specialized toolset created for use in electronic espionage operations. Primary development of IG is done against the Windows 7 32-bit platform, however, features are implemented in a way that they "should" work on newer versions of Windows as well as the 64-bit windows architecture. The rootkit does not make use of techniques which could trigger the kernel patch protection/patchguard implemented on 64-bit versions of Windows and aims, but does not currently support, being loaded by hFiref0x/EP_X0FF's "Turla Driver Loader" toolkit in order to bypass Driver Signature Enforcement on 64-bit versions of Windows. 
# Building

To build IG you must install the Windows Driver Kit a Windows Style "MakeFile" is provided which can be used to build the driver by using the "build" command provided within the WDK. 

# Debugging

For debugging BSOD/Bugchecks it is useful to utilize windbg kernel debugging to analyze crash dumps located in C:\Windows\MEMORY.dmp as well as DbgView is able to analyze kernel crash dumps so you can view output of debug statements leading up to crash as well. Setting up LiveKd debugger https://samsclass.info/126/proj/p12-kernel-debug-win10.htm
